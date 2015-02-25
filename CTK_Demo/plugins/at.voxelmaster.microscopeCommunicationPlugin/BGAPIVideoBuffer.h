/*
 * BGAPIVideoBuffer.h
 *
 *  Created on: 18.10.2013
 *      Author: florian
 */

#ifndef BGAPIVIDEOBUFFER_H_
#define BGAPIVIDEOBUFFER_H_

#include <QtMultimediaKit/QAbstractVideoBuffer>
#include <vector>
#include <iostream>
#include <fstream>


#include <vector>
using namespace std;


#include <stdlib.h>
#include <stdio.h>

#include "bgapi.hpp"



class BGAPIVideoBuffer : public QAbstractVideoBuffer{
public:

	static vector<QAbstractVideoBuffer*> createQAbstractVideoBuffers();


	BGAPIVideoBuffer(HandleType type,BGAPI::Camera *pCamera);
	virtual ~BGAPIVideoBuffer();


	 MapMode mapMode() const;

	 virtual uchar *map(MapMode mode, int *numBytes, int *bytesPerLine);
	 virtual void unmap();
private:


	BGAPI::Camera *pCamera;

	BGAPI::Image* pImage;


	//BGAPI_RESULT BGAPI_CALLBACK imageCallback( void * callBackOwner, BGAPI::Image* pImage  );

	static int init_systems(int * system_count, vector<BGAPI::System*> * externppSystem)
	{
		BGAPI_RESULT res = BGAPI_RESULT_FAIL;
		int i = 0;

		//this is the base call to find the bgapi_system modules which are necessary to perform any further action
		//every BGPAPI_Function returns a BGAPI_RESULT
		res = BGAPI::countSystems( system_count );

		//You should always check the result to make sure everything works fine
		if( res != BGAPI_RESULT_OK )
		{
			//in case of error you will get a result different from BGAPI_RESULT_OK
			//all resultcodes are defined in bgapiresult.h and are returned for special reasons
			printf( "BGAPI_CountSystems Errorcode: %d system_count %d\n", res, *system_count );
			return res;
		}

		for( i = 0; i < *system_count; i++ )
		{
			BGAPI::System * pSystem = NULL;
			res = BGAPI::createSystem( i, &pSystem );
			if( res != BGAPI_RESULT_OK )
			{
				printf("BGAPI::CreateSystem Errorcode: %d Systemnumber %d SysPointer 0x%p\n", res, i, (void*)pSystem );
				externppSystem->clear();
				return res;
			}

			res = pSystem->open();
			if( res != BGAPI_RESULT_OK )
			{
				printf( "BGAPI::System::open Systemnumber %d Errorcode: %d\n", i, res );
				externppSystem->clear();
				return res;
			}
			externppSystem->push_back( pSystem );
		}
		return res;
	}

	static int init_camera( int system_count, vector<BGAPI::System*> * externppSystem, int * pCurrSystem, BGAPI::Camera ** externppCamera, int cam )
	{
		BGAPI_RESULT res = BGAPI_RESULT_FAIL;

		int camera_count = 0;
		vector<int> cameras;
		vector<int>::iterator camIter;
		BGAPI_FeatureState state;
		BGAPIX_CameraInfo cameradeviceinfo;

		vector<BGAPI::System*>::iterator systemIter;


		camera_count = 0;
		for( systemIter = externppSystem->begin(); systemIter != externppSystem->end(); systemIter++ )
		{

				camera_count++;
				//if( camera_count == inputVal )
				{
					*pCurrSystem = (int)(systemIter - externppSystem->begin());

					//this is an example how to create a camera
					res = (*externppSystem)[*pCurrSystem]->createCamera( cam, externppCamera );
					if( res != BGAPI_RESULT_OK )
					{
						printf("\n");
						printf( "BGAPI::System::createCamera Systemnumber %d Errorcode: %d\n", systemIter - externppSystem->begin(), res );
						return res;
					}

					//this is an example how to open a camera
					res = (*externppCamera)->open();
					if( res != BGAPI_RESULT_OK )
					{
						printf("\n");
						printf( "BGAPI::Camera::open Systemnumber %d Errorcode: %d\n", *pCurrSystem, res );
						return res;
					}
					break;
				}

		}
		return res;
	}

	int release_systems( vector<BGAPI::System*> * externppSystem )
	{
		BGAPI_RESULT res = BGAPI_RESULT_FAIL;
		vector<BGAPI::System*>::iterator systemIter;

		for( systemIter = externppSystem->begin(); systemIter != externppSystem->end(); systemIter++ )
		{
			res = (*systemIter)->release();
			if( res != BGAPI_RESULT_OK )
			{
				printf( "BGAPI::System::release %d Errorcode: %d\n", systemIter - externppSystem->begin(), (int)res );
			}
		}
		externppSystem->clear();
		return res;
	}
	int release_images( vector<BGAPI::Image*> * ppImage )
	{
		BGAPI_RESULT res = BGAPI_RESULT_FAIL;
		vector<BGAPI::Image*>::iterator imageIter;
		bool tmpExtern = false;
		unsigned char* tmpBuffer = NULL;

		for( imageIter = ppImage->begin(); imageIter != ppImage->end(); imageIter++ )
		{
			res = ((BGAPI::Image*)(*imageIter))->isExternBuffer( &tmpExtern );
			if( res != BGAPI_RESULT_OK )
			{
				printf( "BGAPI::Image::isExternBuffer %d Errorcode: %d\n", imageIter - ppImage->begin(), (int)res );
			}

			if( tmpExtern )
			{
				res = ((BGAPI::Image*)(*imageIter))->getBuffer( &tmpBuffer );
				if( res != BGAPI_RESULT_OK )
				{
					printf( "BGAPI::Image::getBuffer %d Errorcode: %d\n", imageIter - ppImage->begin(), (int)res );
				}
				else
				{
					free( tmpBuffer );
				}
			}
			res = BGAPI::releaseImage( *imageIter );
			if( res != BGAPI_RESULT_OK )
			{
				printf( "BGAPI::releaseImage %d Errorcode: %d\n", imageIter - ppImage->begin(), (int)res );
			}
		}
		ppImage->clear();
		return res;
	}

};



#endif /* BGAPIVIDEOBUFFER_H_ */
