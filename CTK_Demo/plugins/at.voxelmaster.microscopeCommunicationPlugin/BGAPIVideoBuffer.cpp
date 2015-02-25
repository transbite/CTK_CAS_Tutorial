/*
 * BGAPIVideoBuffer.cpp
 *
 *  Created on: 29.10.2013
 *      Author: florian
 */

#include "BGAPIVideoBuffer.h"
#include <QLibrary>

/*
BGAPI_RESULT BGAPI_CALLBACK imageCallback( void * callBackOwner, BGAPI::Image* pImage  )
	{
		BGAPI_RESULT res = BGAPI_RESULT_OK;

		unsigned char* buffer = NULL;
		pImage->get( &buffer );
		//Now you have the Imagebuffer and can do with it whatever you want

		int swc = 0;
		int hwc = 0;
		pImage->getNumber( &swc, &hwc );

				printf("Image number %d.\r", swc );
		fflush( stdout );

		pImage->getImageLength(numBytes);
						printf("numBytes %d \n",*numBytes);
						std::cout << "4444444444444444444444444444" << std::endl;
						int width = 0;
						int height = 0;
						std::cout << "5555555555555555555555555555555" << std::endl;
						pImage->getSize(&width,&height);
						std::cout << "666666666666666666666666666666666" << std::endl;
						printf("size %d %d\n",width,height);
						std::cout << "77777777777777777777777777777777777" << std::endl;
						*bytesPerLine = *numBytes/height;


		//after you are ready with this image, return it to the camera for the next image
		res = ((BGAPI::Camera*)callBackOwner)->setImage( pImage );
		if( res != BGAPI_RESULT_OK )
			printf( "setImage failed with %d\n", res );

		return res;
	}
*/
vector<QAbstractVideoBuffer*> BGAPIVideoBuffer::createQAbstractVideoBuffers(){

							vector<QAbstractVideoBuffer*> cameraBuffers;

							QLibrary myLib("libbgapi.so");
							myLib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
							myLib.loadHints();
							myLib.load();

							if(myLib.errorString().contains("No such file")){
								fprintf( stdout, "Error: %s\n",myLib.errorString().toStdString().c_str() );
							}else{

									BGAPI::Camera *pNewCamera;

									int system_count = 0;
									int currSystem = 0;

									int inputVal = -1;

									vector<BGAPI::System*> ppSystem;


									BGAPI_RESULT res = BGAPI_RESULT_FAIL;

									printf("Initialize BGAPI Cameras...\n");

									//init the system like shown in bgapi_init example
									res = init_systems( &system_count, &ppSystem );
									if( res != BGAPI_RESULT_OK )
									{
										printf("init_systems Errorcode: %d\n", res);

									}else {



											int count = 0;
											//this is an example how to count available cameras for all available systems
											res = ppSystem.data()[0]->countCameras( &count );


										//select a camera to use
										for(int camNumber=0;camNumber<count;camNumber++){
											res = init_camera( system_count, &ppSystem, &currSystem, &pNewCamera, camNumber );
											if(res == BGAPI_RESULT_OK){
												cameraBuffers.push_back(new BGAPIVideoBuffer(BGAPIVideoBuffer::NoHandle,pNewCamera));
											}
										}
							}
							}
							return cameraBuffers;

};

BGAPIVideoBuffer::BGAPIVideoBuffer(HandleType type,BGAPI::Camera *pNewCamera):QAbstractVideoBuffer(type) {
							this->pCamera=pNewCamera;
							BGAPIX_TypeListINT listint;
							BGAPIX_TypeINT intNumberofbuffer;
							BGAPI_RESULT res = BGAPI_RESULT_FAIL;
							BGAPI_FeatureState state;
							BGAPIX_TypeRangeINT dataLen;

							listint.cbSize = sizeof( BGAPIX_TypeListINT );
							state.cbSize = sizeof( BGAPI_FeatureState );
							intNumberofbuffer.cbSize = sizeof( BGAPIX_TypeINT );
							dataLen.cbSize = sizeof ( BGAPIX_TypeRangeINT );

	/*
							res = pCamera->setDataAccessMode(BGAPI_DATAACCESSMODE_NORMALEXTERN, 20);
							if (res != BGAPI_RESULT_OK)
							{
								printf("getDataAccessMode returned with errorcode %d\n", res);
							}
	*/
							BGAPIX_CameraPixelFormat pformat;
							pformat.cbSize = sizeof ( BGAPIX_CameraPixelFormat );
							BGAPIX_TypeListINT formatlist;
							formatlist.cbSize = sizeof( BGAPIX_TypeListINT );
							int formatindex = 1;
							BGAPIX_TypeListINT pixellist;
							pixellist.cbSize = sizeof( BGAPIX_TypeListINT );

							res = pCamera->getImageFormat( &state, &formatlist );

							BGAPIX_CameraImageFormat cformat;
							cformat.cbSize = sizeof ( BGAPIX_CameraImageFormat );



							for( int i = 0; i<formatlist.length; i++ )
							{
								res = pCamera->getImageFormatDescription( i, &cformat );

								if( res != BGAPI_RESULT_OK ){
									printf("BGAPI::Camera::getImageFormatDescription Errorcode: %d\n", res);
								}

								if(i==formatlist.current){

								printf("         %s: %dx%d \n"
										, (char*)cformat.sName, cformat.iSizeX,
										cformat.iSizeY);
								}

							}


							pCamera->setPixelFormat(BGAPI_PIXTYPE_RGB8_PACKED);
							res = pCamera->getPixelFormat( formatindex, &state, &pixellist );
							if( res != BGAPI_RESULT_OK )
							{
								printf("BGAPI::Camera::getPixelFormat Errorcode: %d\n", res);

							}


						for( int p = 0; p<pixellist.length; p++ )
							{

								res = pCamera->getPixelFormatDescription( formatlist.current, pixellist.array[p], &pformat );



								if( res != BGAPI_RESULT_OK )
								{
									printf("BGAPI::Camera::getPixelFormatDesc Errorcode: %d\n",
												res);




								}else{
									if(p==pixellist.current)
									printf("         %s | %d bit \n",
										(char*)pformat.sName, pformat.iPixelBits);
								}
							}

							res = pCamera->setImagePolling( true );
								if( res != BGAPI_RESULT_OK )
								{
									printf( "Error %d while set Image polling.\n", res );
								}

							//create image object
									res = BGAPI::createImage( &pImage );
									if( res != BGAPI_RESULT_OK )
									{
										printf("BGAPI::createImage for Image 0 returned with errorcode %d\n", res );
									}

	/*
										//allocate memory
										res = pCamera->getDataLength( &state, &dataLen );
										if( res != BGAPI_RESULT_OK )
										{
											printf("BGAPI::Camera::getDataLength for Image 0 returned with errorcode %d\n", res );
										}

										imagebuffer = (unsigned char*) malloc( dataLen.current );

										res = pImage->setBuffer( imagebuffer, dataLen.current );
										if( res != BGAPI_RESULT_OK )
										{
											printf("BGAPI::Image::setBuffer for Image 0 returned with errorcode %d\n", res );
										}
	*/

									res = pCamera->setImage( pImage );
									if( res != BGAPI_RESULT_OK )
									{
										printf("BGAPI::Camera::setImage for Image 0 returned with errorcode %d\n", res );
									}
									res = pCamera->setStart( true );

									/*			res = pCamera->registerNotifyCallback( pCamera, (BGAPI::BGAPI_NOTIFY_CALLBACK)&imageCallback );
										if( res != BGAPI_RESULT_OK )
										{
											printf( "Error %d while register NotifyCallback.\n", res );
										}

										imagebuffer = NULL;
										res = pCamera->setStart( true );
	*/

	};
 BGAPIVideoBuffer::~BGAPIVideoBuffer(){
	 if(pCamera!=NULL)
		 pCamera->setStart( false );
 };


 QAbstractVideoBuffer::MapMode BGAPIVideoBuffer::mapMode() const{
	return   QAbstractVideoBuffer::NotMapped;
 };

 uchar *BGAPIVideoBuffer::map(MapMode mode, int *numBytes, int *bytesPerLine){

	// std::cout << "Microscope Communication map buffer called" << std::endl;

		BGAPI_RESULT res = BGAPI_RESULT_FAIL;
		int receiveTimeout = 1000;
		//res = pCamera->setStart( true );

		unsigned char* imagebuffer = NULL;

		res = pCamera->getImage( &pImage, receiveTimeout );
				if( res != BGAPI_RESULT_OK )
				{
					printf("BGAPI_Camera_getImage returned with errorcode %d\n", res );
				}
				else
				{

					res = pImage->get( &imagebuffer );

					//Now you have the Imagebuffer and can do with it whatever you want
					int swc = 0;
					int hwc = 0;

					pImage->getNumber( &swc, &hwc );


					pImage->getImageLength(numBytes);
					//printf("numBytes %d \n",*numBytes);

					int width = 0;
					int height = 0;

					pImage->getSize(&width,&height);
					//printf("size %d %d\n",width,height);
					*bytesPerLine = *numBytes/height;

					//after you are ready with this image, return it to the camera for the next image
					res = pCamera->setImage( pImage );
					if( res != BGAPI_RESULT_OK )
					{
						printf( "setImage failed with %d\n", res );
					}
				}


		//std::cout << "Microscope Communication map buffer ended" << std::endl;

		return imagebuffer;
	};

 void BGAPIVideoBuffer::unmap(){
	// std::cout << "Microscope Communication UNMAP buffer ended" << std::endl;
		//free(pImage);
	};
