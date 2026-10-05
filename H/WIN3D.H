//------------------------------------------------------------------------------
//Filename       win3d.h
//System         
//Author         Dave Whiteside
//Date           Thu 30 Jan 1997
//Description    
//------------------------------------------------------------------------------
#pragma once

#define	DEFAULT_WIN3D 0

#include "DOSDEFS.H"
#include <assert.h>
#include "WIN32_COMPAT.H"
#include "ddraw_stub.h" // RERUN stubbed version
//RERUN #include <d3d.h>
#include "d3d_stub.h"
#pragma pack(1)														  //JIM 30/07/99
#include "MYSTACK.H"
#include "AREATYPE.H"

//RERUN tryout #include "D3DMACS.H"
#include <vulkan/vulkan.h>
#include "direct_3d.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <iostream>
#include <memory>    // for std::unique_ptr and std::make_unique

extern UByte videoBuffer; //RERUN
#define SDL_PIXELBITCOUNT 16 //RERUN for SDL

// RERUN Overlay message queue
struct OverlayMsg {
    int x;
    int y;
    std::string text;
};

class direct_draw;

typedef class direct_draw DirectDD,*DirectDDP;
typedef class direct_3d Direct3D,*Direct3DP;

typedef int	HTEXTURE;	//Rowan texture handle

const HTEXTURE R3DERR_CREATETEXTURE_FAILED 	= -1;
const HTEXTURE R3DERR_OUTOFTEXTURES 		= -2;

class direct_draw
{
	protected:

	//////////////////////// VULKAN

	VkInstance        vkInstance = VK_NULL_HANDLE;
	VkPhysicalDevice  vkPhys = VK_NULL_HANDLE;
	VkDevice          vkDevice = VK_NULL_HANDLE;
	VkQueue           vkQueue = VK_NULL_HANDLE;
	uint32_t          vkQueueFamily = 0;
	VkQueue           vkPresentQueue = VK_NULL_HANDLE;
	uint32_t          vkPresentFamily = 0;

	VkSurfaceKHR      vkSurface = VK_NULL_HANDLE;
	VkSwapchainKHR    vkSwapchain = VK_NULL_HANDLE;
	VkExtent2D        vkExtent = {0,0};
	VkFormat          vkSwapFormat = VK_FORMAT_UNDEFINED;

	VkImage*          swapImages = nullptr;
	VkImageView*      swapViews = nullptr;
	std::vector<VkImageView>    swapImageViews {};
	uint32_t          swapCount = 0;

	VkRenderPass      renderPass = VK_NULL_HANDLE;
	VkPipelineLayout  pipelineLayout = VK_NULL_HANDLE;
	VkPipeline        pipeline = VK_NULL_HANDLE;

	VkCommandPool     cmdPool = VK_NULL_HANDLE;
	VkCommandBuffer   cmd = VK_NULL_HANDLE;

	VkSemaphore       semAcquire = VK_NULL_HANDLE; // image available
	VkSemaphore       semPresent = VK_NULL_HANDLE; // render finished
	VkFence           fence = VK_NULL_HANDLE;

	/* Game framebuffer */
	VkImage           fbImage = VK_NULL_HANDLE;
	VkDeviceMemory    fbMemory = VK_NULL_HANDLE;
	VkImageView       fbView = VK_NULL_HANDLE;
	VkSampler         fbSampler = VK_NULL_HANDLE;

	/* Upload buffer */
	VkBuffer          stagingBuf = VK_NULL_HANDLE;
	VkDeviceMemory    stagingMem = VK_NULL_HANDLE;
	void*             stagingPtr = nullptr;

	/* Descriptor */
	VkDescriptorSetLayout descLayout = VK_NULL_HANDLE;
	VkDescriptorPool      descPool = VK_NULL_HANDLE;
	VkDescriptorSet       descSet = VK_NULL_HANDLE;

	int g_winWidth = 0, g_winHeight = 0;

	std::vector<VkFramebuffer> framebuffers;

	public:
	void XX_ScreenFlip_Vulkan(SDL_Surface *ddsBack);
	void VulkanPrint(SWord& x,SWord& y, const char* str);

	protected:
	uint32_t FindMemory(uint32_t mask, VkMemoryPropertyFlags flags);
	uint32_t* readSpirvRaw(const char* path, size_t* outBytes);
	VkShaderModule loadShader(VkDevice device, const char* path);
	bool Vulkan_Init(HWND hWnd); //(SDL_Window* theWin);
	bool CreateRenderPass();
	bool CreateFramebuffers();
	void createDescriptorSetLayoutAndSampler();
	void createPipelineLayout();
	SDL_bool Vulkan_CreateSDLSurface(SDL_Window *theWin);
	void Vulkan_Shutdown();
	void Vulkan_ShutdownGuarded();	// bounded variant, safe at teardown
	static void* Vulkan_ShutdownWorker(void*);	// helper for Guarded

	bool Vulkan_CreatePipeline();
	//////////////////////// VULKAN


//------------------------------------------------------------------------------
//	STRUCTURE/CLASS DEFINITIONS
//------------------------------------------------------------------------------
//
// D3DAppD3DDriver structure
// Describes a D3D driver
//
	typedef
	struct D3DAppDDDriver
	{
    	char Name[30];      	/* short name of the driver */
	    char About[50];     /* short string about the driver */
	    GUID Guid;          /* it's GUID */
		ULong vidMem;		/* Total RAM available on the card */
	    BOOL bHardware3D;   /* does it have 3d hardware assist */
		BOOL bCanBltSysMem;	/* can this device support blitting to/from system memory */
		BOOL bDoesAsyncBlt; /* can it do asynchronous blit operations? */
	}
	DDAppD3DDriver;

//
// D3DAppD3DModes structure
// Describes graphics modes
// supported by a D3D driver
//
	typedef
	struct D3DAppD3DModes
	{
		int		Driver;		// -1 == Primary directdraw driver
		int		Width,		// width
				Height,		// height
				Bpp;		// colour depth

		DWORD	dwRedBits,	dwRedShift;
		DWORD	dwGreenBits,dwGreenShift;
		DWORD	dwBlueBits,	dwBlueShift;
	}
	D3DAppD3DModes;

//------------------------------------------------------------------------------
//	MEMBER VARIABLE DEFINITIONS
//------------------------------------------------------------------------------

	protected:

//SDL	LPDIRECTDRAW		lpDD;
	SDL_Renderer* lpDD;

	D3DAppDDDriver		Drivers[D3DAPP_MAXDDDRIVERS];
	D3DAppDDDriver		ThisDriver;
	D3DAppD3DModes		Modes[D3DAPP_MAXD3DMODES];
	D3DAppD3DModes		ThisMode;

	int	NumDrivers;
	int CurrDriver;
	int	NumModes;
	int CurrMode;
	int	CanDoHardware3D;

	ULong primaryDisplayDriverMem;
	BOOL bPrimaryDisplayDriverDoesHw3D;
	BOOL bButNotVeryWell;
	BOOL bFogVertexMode;
	BOOL bFogTableMode;

	BOOL primaryDoesAsyncBlt;
	BOOL primaryCanBltSysMem;

	public:

	UByte* softpallookup;

//SDL	LPDIRECTDRAW2	lpDD2;
	SDL_Renderer*	lpDD2;

	bool fullScreenRequired;

//------------------------------------------------------------------------------
//	MEMBER FUNCTION DEFINITIONS
//------------------------------------------------------------------------------

	SWord				lockLevel;
	bool				gotShifts;
	ROWANSURFACEDESC	rsdGlobal;
	UWord				swRedMask,swGreenMask,swBlueMask;
	SWord				swRedSh1, swGreenSh1, swBlueSh1;
	SWord				swRedSh2, swGreenSh2, swBlueSh2;


	public:
  	
	CON	direct_draw();
	DES ~direct_draw();

	void MakePaletteFadeTable(struct _DirectDraw*,UByte*,UWord*,UByte,UByte,UByte);
	void MakeColourRangeFadeTable(struct _DirectDraw*,UWord*,UByte,UByte,UByte,UByte,UByte,UByte);
	void LockScr(struct _DirectDraw*,ROWANSURFACEDESC*);
	void UnlockScr(struct _DirectDraw*,ROWANSURFACEDESC*);
	void GetColourInfo(struct _DirectDraw*,
						UWord&,SWord&,SWord&,
						UWord&,SWord&,SWord&,
						UWord&,SWord&,SWord&);
	void GetSurfaceDimensions(struct _DirectDraw*,SLong&,SLong&);
	void SetSmackerMode(bool flag,HWND wnd);

	void GetModes(_DirectDraw&);

	BOOL SetMode(int mode);

	BOOL DriverDoesHardware3D();

	Bool ModeSupported(ScreenRez*);

	Bool DriverIsCapable(SWord);

	bool isFullScreen() {return fullScreenRequired;}

	void GetBltCaps(bool&,bool&);

	UByte* NameNumberedDriver(SWord);

	SWord CountDrivers();										//PD 27May97

	protected:

	static BOOL FAR PASCAL enumDDDrivers(	GUID FAR*,	LPSTR,
											LPSTR, 		LPVOID);
	static HRESULT WINAPI enumDisplayModes(	LPDDSURFACEDESC,	
											LPVOID);
	static HRESULT WINAPI enum3DDevices(	LPGUID,			LPSTR,
											LPSTR,			LPD3DDEVICEDESC,
											LPD3DDEVICEDESC,LPVOID);
};

class CPolyTree;

// Macro that should dump an objects current reference count
// to the debugger log window

#define DEBUGREFCOUNT(lpInterface)

#undef	RELEASE


#define	RELEASE(lpInterface,hrError)					\
		if ((lpInterface)!=NULL){						\
			DEBUGREFCOUNT((lpInterface));				\
			(hrError) =									\
			(lpInterface)->Release();					\
			(lpInterface)=NULL;							\
		}												\

#define	RELEASE_SURFACE(lpInterface,hrError)					\
		if ((lpInterface)!=NULL){						\
			SDL_FreeSurface(lpInterface);					\
			(lpInterface)=NULL;							\
		}

#define	RELEASE_RENDERER(lpInterface,hrError)					\
		if ((lpInterface)!=NULL){						\
			SDL_DestroyRenderer(lpInterface);					\
			(lpInterface)=NULL;							\
				}

#define	RELEASE_TEXTURE(lpInterface,hrError)					\
		if ((lpInterface)!=NULL){						\
			SDL_DestroyTexture(lpInterface);					\
			(lpInterface)=NULL;							\
						}

#ifndef	NDEBUG

#define	DEBUGTEXT(p1)			\
		OutputDebugString(p1);

#else

#define	DEBUGTEXT(p1)

#endif


