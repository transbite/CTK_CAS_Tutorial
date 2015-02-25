#include "optotrak.h"
#include "ndopto.h"

#include <QLibrary>
#include "trackerCommunicationPlugin.h"



void optotrak::run(){
		 std::cout<<"Thread RUN optotrak.h"<<std::endl;



		 OptotrakSettings
		 		dtSettings;
		     int
		 		i,
		 		nIterations,
		 		nCurDevice,
		 		nCurMarker,
		 		nDevices,
		 		nMarkersToActivate;
		 	ApplicationDeviceInformation
		 		*pdtDevices;
		     DeviceHandle
		         *pdtDeviceHandles;
		     DeviceHandleInfo
		         *pdtDeviceHandlesInfo;
		 	unsigned int
		 		uFlags,
		 		uElements,
		 		uFrameNumber;
		 	RigidBodyDataType
		 	        RigidBodyData;
		     char
		         szNDErrorString[MAX_ERROR_STRING_LENGTH + 1];

		     /*
		      * initialization
		 	 * intialize variables
		      */
		 	pdtDevices = NULL;
		 	pdtDeviceHandles = NULL;
		 	pdtDeviceHandlesInfo = NULL;
		 	dtSettings.nMarkers = 4;
		 	dtSettings.fFrameFrequency = SAMPLE_FRAMEFREQ;
		 	dtSettings.fMarkerFrequency = SAMPLE_MARKERFREQ;
		 	dtSettings.nThreshold = 30;
		 	dtSettings.nMinimumGain = 160;
		 	dtSettings.nStreamData = SAMPLE_STREAMDATA;
		 	dtSettings.fDutyCycle = SAMPLE_DUTYCYCLE;
		 	dtSettings.fVoltage = SAMPLE_VOLTAGE;
		 	dtSettings.fCollectionTime = 5.0;
		 	dtSettings.fPreTriggerTime = 0.0;
		 	nDevices = 1;
		 	nMarkersToActivate = 4;



		 		fprintf( stdout, "...TransputerLoadSystem\n" );

		 		 QLibrary myLib("liboapi.so");
		 		 myLib.setLoadHints(QLibrary::ExportExternalSymbolsHint);
		 		myLib.loadHints();
		 		myLib.load();

		 		if(myLib.errorString().contains("No such file")){
		 			fprintf( stdout, "Error: %s\n",myLib.errorString().toStdString().c_str() );
		 		}else{
		 		 typedef int (*MyPrototype)(char *);
		 		 MyPrototype myFunction = (MyPrototype) myLib.resolve("TransputerLoadSystem");
		 		 if (myFunction){

		 		     myFunction("system");

		 		 }



					fprintf( stdout, "...TransputerInitializeSystem\n" );
					 if( TransputerInitializeSystem( OPTO_LOG_ERRORS_FLAG | OPTO_LOG_MESSAGES_FLAG ) != OPTO_NO_ERROR_CODE )
					{
						goto ERROR_EXIT;
					}


					fprintf( stdout, "...DetermineSystem\n" );
					if( uDetermineSystem( ) != OPTOTRAK_CERTUS_FLAG )
					{
						goto ERROR_EXIT;
					}


						/*
						 * Strober Initialization
						 * Once communication has been initialized, the application must
						 * determine the strober configuration.
						 * The application retrieves device handles and all strober
						 * properties from the system.
						 */
						fprintf( stdout, "...DetermineStroberConfiguration\n" );
						if( DetermineStroberConfiguration( &pdtDeviceHandles, &pdtDeviceHandlesInfo, &nDevices ) != OPTO_NO_ERROR_CODE )
						{
							goto ERROR_EXIT;
						} /* if */

						/*
						 * Set optional processing flags (this overides the settings in Optotrak.INI).
						 */
						fprintf( stdout, "...OptotrakSetProcessingFlags\n" );
						if( OptotrakSetProcessingFlags( OPTO_LIB_POLL_REAL_DATA |
														OPTO_CONVERT_ON_HOST |
														OPTO_RIGID_ON_HOST ) )
						{
							goto ERROR_EXIT;
						}

						/*
						 * Load camera parameters.
						 */
						fprintf( stdout, "...OptotrakLoadCameraParameters\n" );
						if( OptotrakLoadCameraParameters( "standard" ) != OPTO_NO_ERROR_CODE )
						{
							goto ERROR_EXIT;
						} /* if */




						if( OptotrakSetupCollection( dtSettings.nMarkers,
													 dtSettings.fFrameFrequency,
													 dtSettings.fMarkerFrequency,
													 dtSettings.nThreshold,
													 dtSettings.nMinimumGain,
													 dtSettings.nStreamData,
													 dtSettings.fDutyCycle,
													 dtSettings.fVoltage,
													 dtSettings.fCollectionTime,
													 dtSettings.fPreTriggerTime,
													 OPTOTRAK_NO_FIRE_MARKERS_FLAG | OPTOTRAK_BUFFER_RAW_FLAG ) != OPTO_NO_ERROR_CODE )
						{
							goto ERROR_EXIT;
						} /* if */
						sleep( 1 );

						/*
										 * Prepare for realtime data retrieval.
										 * Activate markers. Turn on the markers prior to data retrieval.
										 */
										fprintf( stdout, "...OptotrakActivateMarkers\n" );
										if( OptotrakActivateMarkers( ) != OPTO_NO_ERROR_CODE )
										{
											goto ERROR_EXIT;
										} /* if */

										sleep( 1 );

						/*
							 * Add a rigid body for tracking to the Optotrak system from a .RIG file.
							 */
							fprintf( stdout, "...RigidBodyAddFromFile\n" );
							if( RigidBodyAddFromFile(
									RIGID_BODY_ID,  /* ID associated with this rigid body.*/
									1,              /* First marker in the rigid body.*/
									"testpivot",        /* RIG file containing rigid body coordinates.*/
								   OPTOTRAK_QUATERN_RIGID_FLAG  | OPTOTRAK_RETURN_QUATERN_FLAG | OPTOTRAK_DO_RIGID_CALCS_FLAG) )           /* Flags. */
							{
								goto ERROR_EXIT;
							} /* if */

						/*
						 * Wait one second to let the camera adjust.
						 */
						sleep( 1 );


						std::cout << "Optotrak started!" << std::endl;
						/*
						 * loop around until a device is plugged or unplugged
						 */
						while( 1 )
						{
							//fprintf( stdout, "\nCHANGE THE STROBER CONFIGURATION TO END THIS TEST\n\n" );
							usleep( 100000 );

							if( DataGetLatestTransforms( &uFrameNumber, &uElements, &uFlags, &RigidBodyData ) != OPTO_NO_ERROR_CODE )
							{
								goto ERROR_EXIT;
							} /* if */




							float xcoords[1];
							float ycoords[1];
							float zcoords[1];
							float xrot[1];
							float yrot[1];
							float zrot[1];


								xcoords[0]=RigidBodyData.pRigidData[0].transformation.quaternion.translation.x;
								ycoords[0]=RigidBodyData.pRigidData[0].transformation.quaternion.translation.y;
								zcoords[0]=RigidBodyData.pRigidData[0].transformation.quaternion.translation.z;
								xrot[0]=RigidBodyData.pRigidData[0].transformation.quaternion.rotation.qx;
								yrot[0]=RigidBodyData.pRigidData[0].transformation.quaternion.rotation.qy;
								zrot[0]=RigidBodyData.pRigidData[0].transformation.quaternion.rotation.qz;

		/*

										fprintf( stdout, "\n" );
										fprintf( stdout, "Rigid Body Transformation Data\n\n" );
										fprintf( stdout, "Frame Number: %8u\n", uFrameNumber );
										fprintf( stdout, "Transforms  : %8u\n", uElements );
										fprintf( stdout, "Flags       :   0x%04x\n", uFlags );
								int uRigidCnt=0;
								for( uRigidCnt = 0; uRigidCnt < uElements; ++uRigidCnt )
								{
								   fprintf( stdout, "Rigid Body %u\n",
											 RigidBodyData.pRigidData[ uRigidCnt].RigidId );
									fprintf( stdout, "XT = %8.2f YT = %8.2f ZT = %8.2f\n",
											 RigidBodyData.pRigidData[ uRigidCnt].transformation.
												 euler.translation.x,
											 RigidBodyData.pRigidData[ uRigidCnt].transformation.
												 euler.translation.y,
											 RigidBodyData.pRigidData[ uRigidCnt].transformation.
												 euler.translation.z );
									fprintf( stdout, "Y  = %8.2f P  = %8.2f R  = %8.2f\n",
											 RigidBodyData.pRigidData[ uRigidCnt].transformation.
												 euler.rotation.yaw,
											 RigidBodyData.pRigidData[ uRigidCnt].transformation.
												 euler.rotation.pitch,
											 RigidBodyData.pRigidData[ uRigidCnt].transformation.
												 euler.rotation.roll );
								}
		*//*
								 for( int uRigidCnt = 0; uRigidCnt < uElements; ++uRigidCnt )
										{
								fprintf( stdout, "Rigid Body %u\n",
													 RigidBodyData.pRigidData[ uRigidCnt].RigidId );
											fprintf( stdout, "XT = %8.2f YT = %8.2f ZT = %8.2f\n",
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.translation.x,
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.translation.y,
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.translation.z );
											fprintf( stdout, "Q0 = %8.2f QX = %8.2f QY = %8.2f QZ = %8.2f\n",
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.rotation.q0,
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.rotation.qx,
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.rotation.qy,
													 RigidBodyData.pRigidData[ uRigidCnt].transformation.
														 quaternion.rotation.qz );
										}


										fprintf( stdout, "XT = %8.2f YT = %8.2f ZT = %8.2f\n",
												RigidBodyData.p3dData[0].x,
												RigidBodyData.p3dData[0].y,
												RigidBodyData.p3dData[0].z );
									  fprintf( stdout, "XT = %8.2f YT = %8.2f ZT = %8.2f\n",
										RigidBodyData.p3dData[1].x,
										RigidBodyData.p3dData[1].y,
										RigidBodyData.p3dData[1].z );
									  fprintf( stdout, "XT = %8.2f YT = %8.2f ZT = %8.2f\n",
										RigidBodyData.p3dData[2].x,
										RigidBodyData.p3dData[2].y,
										RigidBodyData.p3dData[2].z );
									  fprintf( stdout, "XT = %8.2f YT = %8.2f ZT = %8.2f\n",
												RigidBodyData.p3dData[3].x,
												RigidBodyData.p3dData[3].y,
												RigidBodyData.p3dData[3].z );

									fprintf( stdout, "Mean XT = %8.2f YT = %8.2f ZT = %8.2f\n",
											(RigidBodyData.p3dData[0].x+RigidBodyData.p3dData[1].x+RigidBodyData.p3dData[2].x+RigidBodyData.p3dData[3].x)/4,
											(RigidBodyData.p3dData[0].y+RigidBodyData.p3dData[1].y+RigidBodyData.p3dData[2].y+RigidBodyData.p3dData[3].y)/4,
											(RigidBodyData.p3dData[0].z+RigidBodyData.p3dData[1].z+RigidBodyData.p3dData[2].z+RigidBodyData.p3dData[3].z)/4
																											);
		*/

							/*
							for( nCurMarker = 0; nCurMarker < dtSettings.nMarkers; nCurMarker++ )
							{




								if( p3dData[nCurMarker].x < MAX_NEGATIVE )
									{
								/
										xcoords[nCurMarker]=(float)-3.0E28;
									}
									else
									{

										xcoords[nCurMarker]=p3dData[nCurMarker].x;
									}

								if( p3dData[nCurMarker].y < MAX_NEGATIVE )
									{

										ycoords[nCurMarker]=(float)-3.0E28;
									}
									else
									{

										ycoords[nCurMarker]=p3dData[nCurMarker].y;
									}

								if( p3dData[nCurMarker].z < MAX_NEGATIVE )
									{

										zcoords[nCurMarker]=(float)-3.0E28;
									}
									else
									{

										zcoords[nCurMarker]=p3dData[nCurMarker].z;
									}
							 }

			*/





							trackerCommunicationPlugin::getInstance()->updateTrackerCoordinates(xcoords,ycoords,zcoords,xrot,yrot,zrot,1);
							/* for */

							/*
							 * check if the tool configuration has changed
							 */
							if( uFlags & OPTO_TOOL_CONFIG_CHANGED_FLAG )
							{
								fprintf( stdout, "\n\n\n...Tool Configuration Changed\n" );
								sleep( 3 );
								break;
							} /* if */
						} /* while */

						sleep( 1 );

						/*
						 * stop the collection since we are finished at this point
						 */
						fprintf( stdout, "...OptotrakStopCollection\n\n\n" );
						if( OptotrakStopCollection( ) != OPTO_NO_ERROR_CODE )
						{
							goto ERROR_EXIT;
						} /* if */



					/*
					 * Exiting the loop above means that the tool configuration has changed
					 */
					fprintf( stdout, "...DetermineStroberConfiguration\n" );
					if( DetermineStroberConfiguration( &pdtDeviceHandles, &pdtDeviceHandlesInfo, &nDevices ) != OPTO_NO_ERROR_CODE )
					{
						goto ERROR_EXIT;
					} /* if */


				 PROGRAM_COMPLETE:
					 /*
					  * CLEANUP
					  */
					fprintf( stdout, "\n" );
					fprintf( stdout, "...TransputerShutdownSystem\n" );
					 OptotrakDeActivateMarkers( );
					 TransputerShutdownSystem( );

					/*
					 * free all memory
					 */
					if( pdtDeviceHandlesInfo )
					{
						for( nCurDevice = 0; nCurDevice < nDevices; nCurDevice++ )
						{
							AllocateMemoryDeviceHandleProperties( &(pdtDeviceHandlesInfo[nCurDevice].grProperties), 0 );
						} /* for */
					} /* if */
					AllocateMemoryDeviceHandles( &pdtDeviceHandles, 0 );
					AllocateMemoryDeviceHandlesInfo( &pdtDeviceHandlesInfo, pdtDeviceHandles, 0 );

					exit( 0 );


				 ERROR_EXIT:
					/*
					 * Indicate that an error has occurred
					 */
					fprintf( stdout, "\nAn error has occurred during execution of the program.\n" );
					 if( OptotrakGetErrorString( szNDErrorString, MAX_ERROR_STRING_LENGTH + 1 ) == 0 )
					 {
						 fprintf( stdout, szNDErrorString );
					 } /* if */

					fprintf( stdout, "\n\n...TransputerShutdownSystem\n" );
					OptotrakDeActivateMarkers( );
					TransputerShutdownSystem( );

					/*
					 * free all memory
					 */
					if( pdtDeviceHandlesInfo )
					{
						for( nCurDevice = 0; nCurDevice < nDevices; nCurDevice++ )
						{
							AllocateMemoryDeviceHandleProperties( &(pdtDeviceHandlesInfo[nCurDevice].grProperties), 0 );
						} /* for */
					} /* if */
					AllocateMemoryDeviceHandles( &pdtDeviceHandles, 0 );
					AllocateMemoryDeviceHandlesInfo( &pdtDeviceHandlesInfo, pdtDeviceHandles, 0 );

					 exit( 1 );
		 		}

	}
