#pragma once

#include "DOSDEFS.H"
#include "d3dx_stub.h"

UByte lastFogRed,lastFogGreen,lastFogBlue;
const D3DCOLOR specularNULL=RGBA_MAKE(0x00,0x00,0x00,0xFF);
const D3DCOLOR White=RGBA_MAKE(0xff,0xff,0xff,0xFF);
D3DCOLOR AmbientLight=White;

class direct_3d;

class CPolyTree
{
    enum {POLY_LIMIT=384,BUFFER_SIZE=65536*4};
    struct SPolyRec
    {
        SPolyRec *lptr,*rptr;
        Float sort_z;
        UByte globalAlphaValue;
        UByte isLightShaded;
        UByte isTranspFade;
        UByte fogRed,fogGreen,fogBlue;
        UByte shadeRed,shadeGreen,shadeBlue;
        POLYTYPE poly_type;
    };

    UByte* pBufferSpace;
    UByte* pBufferCur;
    SLong ppf;
    SPolyRec* pHead;
    direct_3d* pd3d;
    struct _DirectDraw* pDD;

    //------------------------------------------------------------------------------
    //Procedure		DiscardDistantPoly
    //Author		Paul.
    //Date			Fri 11 Dec 1998
    //------------------------------------------------------------------------------
    bool DiscardDistantPoly(Float sz)
    {
        SPolyRec *pLast,*pScan;
        pScan=pHead;
        pLast=NULL;

        while (pScan->rptr!=NULL)
        {
            pLast=pScan;
            pScan=pScan->rptr;
        }
        //pScan is a pointer to the furthest poly away so far

        if (sz>pScan->sort_z)	//if the new poly is further away than the
            return false;		//new one then return false to indicate that the
            //new poly should be discarded

            //check for any polys closer than pScan
            if (pScan->lptr!=NULL)
            {
                if (pLast==NULL)	pHead=pScan->lptr;
                else				pLast->rptr=pScan->lptr;
            }
            else
            {
                if (pLast==NULL)	pHead=NULL;		//RERUN: single-node tree - pLast is NULL here
                else				pLast->rptr=NULL;
            }
            return true;
    }

    //------------------------------------------------------------------------------
    //Procedure		GetPolyRec
    //Author		Paul.
    //Date			Fri 11 Dec 1998
    //------------------------------------------------------------------------------
    SPolyRec* GetPolyRec(LPPOLYTYPE pPoly)
    {
        //don't forget to NULL the left & right pointers
        //					generate a z sort value
        //					backup the vertices of the poly

        SPolyRec* ret_val;
        vertex *vertex_ptr,**vertex_ptr_ptr;
        vertex *source_ptr,**source_ptr_ptr;
        void *void_ptr;

        //allocates RAM for the following data structure...
        //
        //	A SPolyRec structure followed immediately by an array of pointers
        //	to the polygons vertices followed by the polygon
        //	vertices themselves.

        const ULong _SpaceToAllocate=sizeof(SPolyRec)+pPoly->points*(sizeof(vertex)+sizeof(vertex*));
        UByte* _BackupBuffer=pBufferCur;

        if (ULong(pBufferCur)+_SpaceToAllocate-ULong(pBufferSpace)>=BUFFER_SIZE)
            return NULL;

        void_ptr=(void*)pBufferCur;
        pBufferCur+=_SpaceToAllocate;

        ret_val=(SPolyRec*)void_ptr;
        ret_val->lptr=ret_val->rptr=NULL;
        ret_val->poly_type=*pPoly;

        ret_val->poly_type.lplpVertices=
        vertex_ptr_ptr=
        (vertex**)(ULong(ret_val)+sizeof(SPolyRec));

        vertex_ptr=
        (vertex*)(ULong(ret_val)+sizeof(SPolyRec)+ret_val->poly_type.points*sizeof(vertex*));

        source_ptr_ptr=pPoly->lplpVertices;

        UWord andClip=CF3D_ALL;

        Float far_z=0;

        for (SLong x=0;x<pPoly->points;x++)
        {
            source_ptr=*source_ptr_ptr++;
            vertex_ptr_ptr[0]=vertex_ptr;
            vertex_ptr[0]=source_ptr[0];
            if (vertex_ptr[0].bz.f>far_z) far_z=vertex_ptr[0].bz.f;
            vertex_ptr++;
            vertex_ptr_ptr++;
            andClip&=source_ptr[0].clipFlags;
        }

        if (andClip!=CF3D_NULL)
        {
            pBufferCur=_BackupBuffer;
            return NULL;
        }

        ret_val->sort_z=far_z;	//source_ptr[-1].bz.f;

        ppf++;

        if (ppf>POLY_LIMIT && DiscardDistantPoly(ret_val->sort_z)==false)
        {
            pBufferCur=_BackupBuffer;
            return NULL;
        }

        ret_val->globalAlphaValue=pd3d->globalAlphaValue;
        ret_val->isTranspFade=pd3d->isTranspFade?1:0;
        ret_val->isLightShaded=pd3d->isLightShaded?1:0;
        ret_val->fogRed=lastFogRed;
        ret_val->fogGreen=lastFogGreen;
        ret_val->fogBlue=lastFogBlue;
        ret_val->shadeRed=pd3d->vShadeRed;
        ret_val->shadeGreen=pd3d->vShadeGreen;
        ret_val->shadeBlue=pd3d->vShadeBlue;
        return ret_val;
    }

    //------------------------------------------------------------------------------
    //Procedure		RenderRecurse
    //Author		Paul.
    //Date			Mon 14 Dec 1998
    //------------------------------------------------------------------------------
    void RenderRecurse(SPolyRec* next_index)
    {
        SPolyRec* scan_index;

        do
        {
            scan_index=next_index;

            if ((next_index=scan_index->rptr)!=NULL)
                RenderRecurse(next_index);

            pd3d->isTranspFade=scan_index->isTranspFade?true:false;
            pd3d->isLightShaded=scan_index->isLightShaded?true:false;
            pd3d->globalAlphaValue=scan_index->globalAlphaValue;

            pd3d->vShadeRed=scan_index->shadeRed;
            pd3d->vShadeGreen=scan_index->shadeGreen;
            pd3d->vShadeBlue=scan_index->shadeBlue;
            AmbientLight=RGBA_MAKE(scan_index->shadeRed,scan_index->shadeGreen,scan_index->shadeBlue,0xFF);

            if (scan_index->fogRed!=lastFogRed ||
                scan_index->fogGreen!=lastFogGreen ||
                scan_index->fogBlue!=lastFogBlue)
                pd3d->SetFog(scan_index->fogRed,scan_index->fogGreen,scan_index->fogBlue);

            pd3d->RenderPolyCallback(pDD,&(scan_index->poly_type));

            next_index=scan_index->lptr;
        }
        while (next_index!=NULL);
    }

public:

    CPolyTree(struct _DirectDraw* pdd=NULL)
    {
        pDD=pdd;
        pHead=NULL;
        ppf=0;
        pBufferSpace=new UByte[BUFFER_SIZE];
        pBufferCur=pBufferSpace;
    }

    ~CPolyTree()
    {
        delete[] pBufferSpace;
    }

    //------------------------------------------------------------------------------
    //Procedure		AddPoly
    //Author		Paul.
    //Date			Fri 11 Dec 1998
    //------------------------------------------------------------------------------
    void AddPoly(LPPOLYTYPE pPoly)
    {
        bool		inserted=false;

        SPolyRec 	*new_index,
        *next_index,
        *scan_index;

        new_index=GetPolyRec(pPoly);

        if (new_index==NULL)
            return;

        if (pHead==NULL)
            pHead=new_index;
        else
        {
            next_index=pHead;

            while (!inserted)
            {
                scan_index=next_index;

                if (scan_index->sort_z>new_index->sort_z)
                {
                    if ((next_index=scan_index->lptr)==NULL)
                    {
                        scan_index->lptr=new_index;
                        inserted=true;
                    }
                }
                else
                {
                    if ((next_index=scan_index->rptr)==NULL)
                    {
                        scan_index->rptr=new_index;
                        inserted=true;
                    }
                }
            }
        }
    }

    //------------------------------------------------------------------------------
    //Procedure		EnumPolys
    //Author		Paul.
    //Date			Fri 11 Dec 1998
    //------------------------------------------------------------------------------
    void RenderPolys(direct_3d* d3dptr)
    {
        //backup some flags
        SLong bupShadeRed,bupShadeGreen,bupShadeBlue;
        bool isTranspFade,isLightShaded;
        UByte globalAlphaValue;
        pd3d=d3dptr;
        isTranspFade=pd3d->isTranspFade;
        isLightShaded=pd3d->isLightShaded;
        globalAlphaValue=pd3d->globalAlphaValue;
        bupShadeRed=pd3d->vShadeRed;
        bupShadeGreen=pd3d->vShadeGreen;
        bupShadeBlue=pd3d->vShadeBlue;
        D3DCOLOR bupAmbientLight=AmbientLight;

        if (pHead!=NULL)
        {
            RenderRecurse(pHead);
            pHead=NULL;
        }
        ppf=0;
        pBufferCur=pBufferSpace;

        //restore some flags
        pd3d->isTranspFade=isTranspFade;
        pd3d->isLightShaded=isLightShaded;
        pd3d->globalAlphaValue=globalAlphaValue;
        pd3d->vShadeRed=bupShadeRed;
        pd3d->vShadeGreen=bupShadeGreen;
        pd3d->vShadeBlue=bupShadeBlue;
        AmbientLight=bupAmbientLight;
    }
};

