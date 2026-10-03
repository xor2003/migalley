/*
	 MiG Alley
	 Copyright (C) 1998, 1999, 2000, 2001 Empire Interactive (Europe) Ltd,
	 677 High Road, North Finchley, London N12 0DA

	 Please see the document licence.doc for the full licence agreement

2. LICENCE
 2.1 	
 	Subject to the provisions of this Agreement we now grant to you the 
 	following rights in respect of the Source Code:
  2.1.1 
  	the non-exclusive right to Exploit  the Source Code and Executable 
  	Code on any medium; and 
  2.1.2 
  	the non-exclusive right to create and distribute Derivative Works.
 2.2 	
 	Subject to the provisions of this Agreement we now grant you the
	following rights in respect of the Object Code:
  2.2.1 
	the non-exclusive right to Exploit the Object Code on the same
	terms and conditions set out in clause 3, provided that any
	distribution is done so on the terms of this Agreement and is
	accompanied by the Source Code and Executable Code (as
	applicable).

3. GENERAL OBLIGATIONS
 3.1 
 	In consideration of the licence granted in clause 2.1 you now agree:
  3.1.1 
	that when you distribute the Source Code or Executable Code or
	any Derivative Works to Recipients you will also include the
	terms of this Agreement;
  3.1.2 
	that when you make the Source Code, Executable Code or any
	Derivative Works ("Materials") available to download, you will
	ensure that Recipients must accept the terms of this Agreement
	before being allowed to download such Materials;
  3.1.3 
	that by Exploiting the Source Code or Executable Code you may
	not impose any further restrictions on a Recipient's subsequent
	Exploitation of the Source Code or Executable Code other than
	those contained in the terms and conditions of this Agreement;
  3.1.4 
	not (and not to allow any third party) to profit or make any
	charge for the Source Code, or Executable Code, any
	Exploitation of the Source Code or Executable Code, or for any
	Derivative Works;
  3.1.5 
	not to place any restrictions on the operability of the Source 
	Code;
  3.1.6 
	to attach prominent notices to any Derivative Works stating
	that you have changed the Source Code or Executable Code and to
	include the details anddate of such change; and
  3.1.7 
  	not to Exploit the Source Code or Executable Code otherwise than
	as expressly permitted by  this Agreement.

questions about this file may be asked at http://www.simhq.com/
*/

//------------------------------------------------------------------------------
//Filename       winsmack.cpp
//System         Mig Alley
//Author         Dave Whiteside
//Date           Sun 19 Jul 1998
//Description    Smacker interface for migalley
// getfreediskspace
//------------------------------------------------------------------------------
#include "WIN32_COMPAT.H" //RERUN
#include "SDL.h"
#define DIRECTINPUT_VERSION 0x700 // RERUN
#include "dinput_stub.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include	"DOSDEFS.H"
#include	"DISPLAY.H"
#include	"MYERROR.H"
#include	"SMACK.H"
#include	"FILEMAN.H"
#include	"WORLDINC.H"
#include	"MILES.H"
#include	"WINMOVE.H"
#include    "MFC_stub.h"

// Globals for ffmpeg playback
static FILE* ffmpeg_pipe = nullptr;
static FILE* ffmpeg_audio_pipe = nullptr;
static SDL_AudioDeviceID audio_dev = 0;
static Smack* smk_info = nullptr;
static std::vector<unsigned char> frame_buffer;
static int frame_width = 0;
static int frame_height = 0;
static int frame_size = 0;
static Uint32 last_frame_time = 0;

SmackBuf* sbuf = nullptr;
Smack* smk=nullptr;
int			Xoff=0;
int			Yoff=0;
static int Woff=0;
static int Hoff=0;
int			wait=0;

/*
	UWord	DoSmack(int h)
	Returns Number of frames left [0 if finished]
	Inputs	MigWindow Handle


*/
extern	int	introsmk;

// Any-key video interrupt: arms the one-shot skip flag that DoSmack()
// consumes on its next call. Gated on smk so a keypress with no video
// playing can't leave a stale skip armed for the next OpenSmack().
void	InterruptSmack()
{
	if (smk)
		introsmk = 1;
}

#ifdef MIG_DEMO_VER
extern DPlay _DPlay;
#endif

void	CloseSmack()
{
	if (ffmpeg_pipe) {
		pclose(ffmpeg_pipe);
		ffmpeg_pipe = nullptr;
	}
	if (ffmpeg_audio_pipe) {
		pclose(ffmpeg_audio_pipe);
		ffmpeg_audio_pipe = nullptr;
	}
	if (audio_dev) {
		SDL_CloseAudioDevice(audio_dev);
		audio_dev = 0;
	}
	if (smk_info) {
		delete smk_info;
		smk_info = nullptr;
	}
	smk = nullptr;
	sbuf = nullptr;
	frame_buffer.clear();
	//std::cout << "CloseSmack called" << std::endl;
}

Smack* OpenSmack(FileNum filenumber,int win, int x, int y,int w, int h)
{
	char	filepath[256];

	Xoff=x;
	Yoff=y;
	Woff=w;
	Hoff=h;
	wait=0;
	last_frame_time = 0;

	CloseSmack();
	if (!filenumber) return nullptr;

	FILEMAN.namenumberedfile(filenumber, filepath);

	// Use ffprobe to get video info
	char command[512];
	snprintf(command, sizeof(command), "ffprobe -v error -select_streams v:0 -show_entries stream=width,height,nb_frames,avg_frame_rate -of default=noprint_wrappers=1:nokey=1 \"%s\"", filepath);

	FILE* ffprobe_pipe = popen(command, "r");
	if (!ffprobe_pipe) {
		std::cerr << "Failed to run ffprobe on " << filepath << std::endl;
		return nullptr;
	}

	int width = 0, height = 0, frames = 0;
	int fr_num = 0, fr_den = 1;
	char frames_str[32], fr_str[32];

	if (fscanf(ffprobe_pipe, "%d", &width) != 1) { width = 0; }
	if (fscanf(ffprobe_pipe, "%d", &height) != 1) { height = 0; }
	if (fscanf(ffprobe_pipe, "%s", frames_str) == 1) {
		if (strcmp(frames_str, "N/A") != 0) {
			sscanf(frames_str, "%d", &frames);
		}
	}
	if (fscanf(ffprobe_pipe, "%s", fr_str) == 1) {
		sscanf(fr_str, "%d/%d", &fr_num, &fr_den);
	}
	pclose(ffprobe_pipe);

	if (width == 0 || height == 0) {
		std::cerr << "Failed to get video dimensions from ffprobe for " << filepath << std::endl;
		return nullptr;
	}

	// Store info in a new Smack struct
	smk_info = new Smack;
	memset(smk_info, 0, sizeof(Smack));
	smk_info->Width = width;
	smk_info->Height = height;
	smk_info->Frames = frames;
	if (fr_num > 0 && fr_den > 0) {
		smk_info->MSPerFrame = (1000 * fr_den) / fr_num;
	} else {
		smk_info->MSPerFrame = 1000 / 15; // Default to 15 fps
	}
	smk_info->FrameNum = 0;

	// Prepare ffmpeg command to decode to raw RGB
	snprintf(command, sizeof(command), "ffmpeg -loglevel quiet -i \"%s\" -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", filepath);

	ffmpeg_pipe = popen(command, "r");
	if (!ffmpeg_pipe) {
		std::cerr << "Failed to run ffmpeg for " << filepath << std::endl;
		delete smk_info;
		smk_info = nullptr;
		return nullptr;
	}

	// Open audio device
	SDL_AudioSpec want, have;
	SDL_zero(want);
	want.freq = 44100;
	want.format = AUDIO_S16LSB;
	want.channels = 2;
	want.samples = 4096;
	audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
	if (audio_dev) {
		SDL_PauseAudioDevice(audio_dev, 0);
		snprintf(command, sizeof(command), "ffmpeg -i \"%s\" -f s16le -ac 2 -ar 44100 -vn -loglevel quiet -", filepath);
		ffmpeg_audio_pipe = popen(command, "r");
		if (ffmpeg_audio_pipe) {
			int fd = fileno(ffmpeg_audio_pipe);
			int flags = fcntl(fd, F_GETFL, 0);
			fcntl(fd, F_SETFL, flags | O_NONBLOCK);
		}
	}

	// Set up frame buffer
	frame_width = width;
	frame_height = height;
	frame_size = width * height * 3;
	frame_buffer.resize(frame_size);

	smk = smk_info;

	std::cout << "OpenSmack: " << filepath << " (" << width << "x" << height << " @ " << (1000.0f/smk_info->MSPerFrame) << "fps)" << std::endl;

	return smk;
}

UWord	DoSmack(int h)
{
	if (introsmk != 0) {
		introsmk = 0;
		return 0;
	}

#ifdef MIG_DEMO_VER
	if (_DPlay.PossibleLobby)
		return 1;
#endif

	if (!ffmpeg_pipe || !smk) {
		return 0;
	}

	// Handle audio
	if (ffmpeg_audio_pipe && audio_dev) {
		Uint32 queued = SDL_GetQueuedAudioSize(audio_dev);
		if (queued < 88200) { // Keep ~0.5s buffered
			unsigned char audio_buf[8192];
			ssize_t bytes = read(fileno(ffmpeg_audio_pipe), audio_buf, sizeof(audio_buf));
			if (bytes > 0) {
				SDL_QueueAudio(audio_dev, audio_buf, bytes);
			}
		}
	}

	Uint32 current_time = SDL_GetTicks();
	if (last_frame_time == 0) {
		last_frame_time = current_time;
	}
	
	if (current_time < last_frame_time + smk->MSPerFrame) {
		if (smk->Frames > 0) return smk->Frames - smk->FrameNum;
		return 1;
	}
	last_frame_time = current_time;

	size_t bytes_read = fread(frame_buffer.data(), 1, frame_size, ffmpeg_pipe);

	if (bytes_read < frame_size) {
		CloseSmack();
		return 0;
	}

	smk->FrameNum++;

	HWND hWnd = (HWND)h;
	CWnd* pWnd = CWnd::FromHandle(hWnd);
	if (!pWnd) {
		CloseSmack();
		return 0;
	}

	CDC* pDC = pWnd->GetDC();
	if (!pDC || !pDC->m_renderer) {
		if (pDC) pWnd->ReleaseDC(pDC);
		if (smk->Frames > 0) return smk->Frames - smk->FrameNum;
		return 1;
	}
	SDL_Renderer* renderer = pDC->m_renderer;

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
		frame_buffer.data(), frame_width, frame_height, 24, frame_width * 3,
		0x0000FF, 0x00FF00, 0xFF0000, 0);

	if (surface) {
		SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
		if (texture) {
			int winW, winH;
			SDL_GetRendererOutputSize(renderer, &winW, &winH);

			// To play at original size, use frame_width and frame_height.
			// Use Xoff/Yoff for position (or center if -1).
			int dstX = Xoff;
			int dstY = Yoff;
			int dstW = frame_width;
			int dstH = frame_height;

			if (Woff == -1)
			{
				dstW = winW;
				dstH = (frame_height * winW) / frame_width;
				dstX = 0;
				dstY = (winH - dstH) / 2;
			}
			else
			{
				if (dstX == -1) dstX = (winW - frame_width) / 2;
				if (dstY == -1) dstY = (winH - frame_height) / 2;
			}

			SDL_Rect dstRect = { dstX, dstY, dstW, dstH };
			
			// By not clearing the renderer, the background image is preserved.
			// The video frame is drawn on top of it.
			SDL_RenderCopy(renderer, texture, NULL, &dstRect);
			SDL_RenderPresent(renderer);
			
			SDL_DestroyTexture(texture);
		}
		SDL_FreeSurface(surface);
	}
	
	pWnd->ReleaseDC(pDC);

	if (smk->Frames > 0 && smk->FrameNum >= smk->Frames) {
		CloseSmack();
		return 0;
	}

	if (smk->Frames > 0) return smk->Frames - smk->FrameNum;
	return 1;
}
