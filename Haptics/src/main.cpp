//Final Project - Rogerio Garcia Nespolo
//Haptics 2019


#include <QHHeadersGLUT.h>
#include <HDU/hduMath.h>
#include <HDU/hduMatrix.h>
#include <HDU/hduLine.h>
#include <Windows.h>
#include <MMSystem.h>
#include <iostream>
#include <cstdio>
#include <ctime>
#include "LoadFile.h"
////openAL files
#include <al.h>
#include <alc.h>

///////////////////////////Global variables
Cursor* PlaygroundCursor;
QHGLUT* DisplayObject;
DeviceSpace* OmniSpace;
TriMesh* iris;
TriMesh* sclera;
TriMesh* pupil;
TriMesh* retina;
TriMesh* retina2;
TriMesh* cornea;
TriMesh* nerve1;
TriMesh* nerve2;
TriMesh* nerve3;
TriMesh* skull;
TriMesh* rightSclera;
TriMesh* rightIris;
static Box* infoBox1;
static Box* infoBox2;


Text* txtTitle;
Text* txtInstructions1;
Text* txtInstructions2;
Text* txtInstructions3;
Text* txtInstructions4;

Text* txtWarning;

/////////fulcrum and vib forces
hduVector3Dd pos; //position of the cursor
hduVector3Dd fulcrumPoint;
HDdouble nominalMaxForce;
HDdouble accumEyeRotation;
hduQuaternion ori;
hduQuaternion unitVector(0, hduVector3Dd(0, 0, 1));// create unit vector along Z axis of stylus
hduVector3Dd force;

hduMatrix globalDragTransform;
bool vibration = false;
bool fulcrum_eff = false;
bool save_Fulcrum_Point = false;
bool rotateEye = false;
bool rotateEyeRight = false;
int count = 0;

bool moveAll = false;

//////////////////////////Registering callback functions
void graphicsCallback(void);
void motionCallback(unsigned int ShapeID);
void button1DownCallback(unsigned int ShapeID);
void button1UpCallback(unsigned int ShapeID);
void button2DownCallback(unsigned int ShapeID);
void button2UpCallback(unsigned int ShapeID);
void touchCallback(unsigned int ShapeID);
void unTouchCallback(unsigned int ShapeID);

void HLCALLBACK startEffectCB(HLcache *cache, void * userData);
void HLCALLBACK computeForceCB(HDdouble force[3], HLcache *cache, void * userData);
void HLCALLBACK stopEffectCB(HLcache *cache, void * userData);


// ***************  Related to Audio  *********************//
// --------- OpenAL variables -----------//
ALCdevice* audioDevice;
ALCcontext* audioContext;
ALenum errorCode = 0;
ALuint helloBuffer;
ALuint helloSource;
ALenum format;
ALint frequency;
ALfloat SourcePos[] = { 0.0,0.0,0.0 };	// Get haptic cursor position
ALfloat SourceVel[] = { 0.0,0.0,0.0 };
ALfloat ListenerPos[] = { 0.0,0.0,0.0 }; //get camera position
ALfloat ListenerVel[] = { 0.0,0.0,0.0 };
ALfloat ListenerOri[] = { 0.0,0.0,-1.0, 0.0, 1.0, 0.0 };
hduVector3Dd Eye;
hduVector3Dd LookAt;
hduVector3Dd UpVector;
hduVector3Dd cursorPosition;
hduVector3Dd nervePosition;
float distanceCursorNerve;
// ----------------------------------//
//compute force variable 
hduVector3Dd totalForce;
hduVector3Dd newPos;



// --------  Load Audio File  --------
LoadFile File1("tool.wav");

// -------- Flag for start and Pause  ------- //
void button2UpCallback(unsigned int ShapeID);
void button2DownCallback(unsigned int ShapeID);
void motionCallback(unsigned int ShapeID);
void graphicsCallback(void);
void touchCallback(unsigned int ShapeID);
void unTouchCallback(unsigned int ShapeID);


// Find camera position
float FOV;
float NearPlane;
float FarPlane;


//////////////////////////////////main
int main(int argc, char *argv[])
{
	
    DisplayObject = new QHGLUT(argc,argv);//create a display window
    OmniSpace = new DeviceSpace;//Find the default Phantom device 
    DisplayObject->tell(OmniSpace);//Tell QuickHaptics about it

	PlaygroundCursor = new Cursor("models/vitrector.stl"); //Declare a cursor
	PlaygroundCursor->setName("cursorCallback");
	PlaygroundCursor->scaleCursor(0.07);//Tell QuickHaptics that the cursor exists
	TriMesh* ModelTriMeshPointer = PlaygroundCursor->getTriMeshPointer();
	DisplayObject->tell(PlaygroundCursor);//Tell QuickHaptics that the cursor exists
	DisplayObject->setBackgroundColor(0, 0, 0);


	infoBox1 = new Box(1.5, 1.5, 0.1);
	infoBox1->setTranslation(3.5, 2, 2);
	infoBox1->setTexture("images/general_view.jpg");
	infoBox1->setUnDraggable();
	DisplayObject->tell(infoBox1);

	infoBox2 = new Box;
	infoBox2->setScale(0.5);
	infoBox2->setTranslation(1.5, 0.2, 0);
	infoBox2->setVisible(false, false);
	DisplayObject->tell(infoBox2);
    
	iris = new TriMesh("Models/eye/iris.3ds");//Load
	iris->setRenderModeFeedback();
	iris->setRotation(hduVector3Dd(0, 1, 0), 90);
	iris->setRotation(hduVector3Dd(0, 0, 1), -90);
	iris->setHapticVisibility(true);
	iris->setStiffness(0.1);
	iris->setFriction(0.1, 0.1);
	iris->setPopthrough(0.01);
	iris->setUnDraggable();
	DisplayObject->tell(iris);

	sclera = new TriMesh("Models/eye/sclera.3ds");//Load
	sclera->setRenderModeFeedback();
	sclera->setUnDraggable();
	sclera->setRotation(hduVector3Dd(0, 1, 0), 90);
	sclera->setRotation(hduVector3Dd(0, 0, 1), -90);
	sclera->setStiffness(0.7);
	sclera->setFriction(0.9, 0.9);
	sclera->setPopthrough(0.06);
	DisplayObject->tell(sclera);

	skull = new TriMesh("Models/skull.3ds");//Load
	skull->setRenderModeFeedback();
	skull->setTexture("Models/eye/blue.jpg");
	skull->setRotation(hduVector3Dd(0, 1, 0), 90);
	skull->setRotation(hduVector3Dd(0, 0, 1), -90);
	DisplayObject->tell(skull);

	retina = new TriMesh("Models/eye/retina.3ds");//Load the retina level that is not safe 
	retina->setRenderModeFeedback();
	retina->setRotation(hduVector3Dd(0, 1, 0), 90);
	retina->setRotation(hduVector3Dd(0, 0, 1), -90);
	retina->setUnDraggable();
	retina->setStiffness(0.9);
	retina->setFriction(0.3, 0.3);
	retina->setPopthrough(0.9);
	retina->setUnDraggable();
	DisplayObject->tell(retina);

	retina2 = new TriMesh("Models/eye/retina2.3ds");//Load the retina touchable
	retina2->setRenderModeFeedback();
	retina2->setRotation(hduVector3Dd(0, 1, 0), 90);
	retina2->setRotation(hduVector3Dd(0, 0, 1), -90);
	retina2->setUnDraggable();
	retina2->setStiffness(0.5);
	retina2->setFriction(0.3, 0.3);
	retina2->setPopthrough(0.06);
	retina2->setUnDraggable();
	DisplayObject->tell(retina2);

	pupil = new TriMesh("Models/eye/pupil.3ds");//Load
	pupil->setRenderModeFeedback();
	pupil->setUnDraggable();
	pupil->setRotation(hduVector3Dd(0, 1, 0), 90);
	pupil->setRotation(hduVector3Dd(0, 0, 1), -90);
	pupil->setUnDraggable();
	pupil->setStiffness(0.1);
	pupil->setFriction(0.1, 0.1);
	pupil->setPopthrough(0.1);
	DisplayObject->tell(pupil);

	nerve1 = new TriMesh("Models/eye/nerve1.3ds");//Load
    nerve1->setRenderModeFeedback();
	nerve1->setTexture("Models/eye/red.jpg");
	nerve1->setRotation(hduVector3Dd(0, 1, 0), 90);
	nerve1->setRotation(hduVector3Dd(0, 0, 1), -90);
	DisplayObject->tell(nerve1);

	nerve2 = new TriMesh("Models/eye/nerve2.3ds");//Load
	nerve2->setRenderModeFeedback();
	nerve2->setTexture("Models/eye/blue.jpg");
	nerve2->setRotation(hduVector3Dd(0, 1, 0), 90);
	nerve2->setRotation(hduVector3Dd(0, 0, 1), -90);
	DisplayObject->tell(nerve2);

	nerve3 = new TriMesh("Models/eye/nerve3.3ds");//Load
	nerve3->setRenderModeFeedback();
	nerve3->setTexture("Models/eye/blue.jpg");
	nerve3->setRotation(hduVector3Dd(0, 1, 0), 90);
	nerve3->setRotation(hduVector3Dd(0, 0, 1), -90);
	DisplayObject->tell(nerve3);

	rightSclera = new TriMesh("Models/eye/rightSclera.3ds");//Load
	rightSclera->setRenderModeFeedback();
	rightSclera->setRotation(hduVector3Dd(0, 1, 0), 90);
	rightSclera->setRotation(hduVector3Dd(0, 0, 1), -90);
	rightSclera->setUnDraggable();
	rightSclera->setStiffness(0.9);
	rightSclera->setFriction(0.1, 0.1);
	rightSclera->setPopthrough(0.06);
	DisplayObject->tell(rightSclera);

	rightIris = new TriMesh("Models/eye/rightIris.3ds");//Load
	rightIris->setRenderModeFeedback();
	rightIris->setRotation(hduVector3Dd(0, 1, 0), 90);
	rightIris->setRotation(hduVector3Dd(0, 0, 1), -90);
	rightIris->setUnDraggable();
	rightIris->setStiffness(0.9);
	rightIris->setFriction(0.1, 0.1);
	rightIris->setPopthrough(0.9);
	DisplayObject->tell(rightIris);


	txtTitle = new Text(20.0, "Vitrector insertion simulator", "models/courier.ttf");
	txtTitle->setTranslation(0, 0.95,0);
	DisplayObject->tell(txtTitle);
	txtTitle->setGraphicVisibility(true);

	txtInstructions1 = new Text(15.0, "Use button 2 to activate the vitrector. When released, button 2 deactivates the effects to easily remove the tool","models/courier.ttf");
	txtInstructions1->setTranslation(0, 0.90, 0);
	DisplayObject->tell(txtInstructions1);
	txtInstructions1->setGraphicVisibility(true);

	txtInstructions2 = new Text(15.0, "Use button 1 to move the entire model while the skull is being touched", "models/courier.ttf");
	txtInstructions2->setTranslation(0, 0.87, 0);
	DisplayObject->tell(txtInstructions2);
	txtInstructions2->setGraphicVisibility(true);

	txtInstructions3 = new Text(15.0, "Follow the instructions of the box about where and how to insert and move the vitrector", "models/courier.ttf");
	txtInstructions3->setTranslation(0, 0.84, 0);
	DisplayObject->tell(txtInstructions3);
	txtInstructions3->setGraphicVisibility(true);

	txtInstructions4 = new Text(10.0, "The right eye is for illustration purpose only (with rotation and fulcrum)", "models/courier.ttf");
	txtInstructions4->setTranslation(0, 0.81, 0);
	DisplayObject->tell(txtInstructions4);
	txtInstructions4->setGraphicVisibility(true);

	txtWarning = new Text(20.0, "Warning!","models/courier.ttf");
	txtWarning->setTranslation(0, 0.04, 0);
	DisplayObject->tell(txtWarning);
	txtWarning->setGraphicVisibility(false);


	/////registering the callback functions
	DisplayObject->preDrawCallback(graphicsCallback);

	OmniSpace->button1DownCallback(button1DownCallback,skull);
	OmniSpace->button1UpCallback(button1UpCallback, sclera);

	OmniSpace->button2DownCallback(button2DownCallback);
	OmniSpace->button2UpCallback(button2UpCallback);

	//callback for the warning of the force applied to the retina
	OmniSpace->touchCallback(touchCallback, retina);
	OmniSpace->touchCallback(touchCallback, skull);
	OmniSpace->touchCallback(touchCallback, sclera);
	OmniSpace->touchCallback(touchCallback, pupil);
	OmniSpace->touchCallback(touchCallback, iris);
	OmniSpace->touchCallback(touchCallback, rightSclera);

	//callback for the damping and fulcrum - sclera
	OmniSpace->unTouchCallback(unTouchCallback, sclera);
	OmniSpace->unTouchCallback(unTouchCallback, rightSclera);

	OmniSpace->startServoLoopCallback(startEffectCB, computeForceCB, stopEffectCB, NULL);

	OmniSpace->motionCallback(motionCallback);


	//***********************   REGARDING SOUND    ********************************//

	// Create an audio device
	audioDevice = alcOpenDevice(NULL); // NULL refers to the default device connected to the computer
	errorCode = alcGetError(audioDevice);

	// Create an audio context
	// One context can have one Listner and Multiple Sources and Buffers
	audioContext = alcCreateContext(audioDevice, NULL);
	alcMakeContextCurrent(audioContext);
	errorCode = alcGetError(audioDevice);

	// Create a Buffer
	alGenBuffers(1, &helloBuffer); // Buffer stores the audio data
	errorCode = alGetError();

	// Create a source
	alGenSources(1, &helloSource); // Virtual source from which the sound is emitted in the application
	errorCode = alGetError();

	// Read information about the WAVE file from the Object "File1" we created
	frequency = File1.sampleRate;
	// Find if file is stereo or mono
	// Find if file is 8 or 16 bit 
	if (File1.bitsPerSample == 8) {
		if (File1.channels == 1)
			format = AL_FORMAT_MONO8;
		else if (File1.channels == 2)
			format = AL_FORMAT_STEREO8;
	}
	else if (File1.bitsPerSample == 16) {
		if (File1.channels == 1)
			format = AL_FORMAT_MONO16;
		else if (File1.channels == 2)
			format = AL_FORMAT_STEREO16;

	}

	// Fill the buffer with the information
	alBufferData(helloBuffer, format, File1.buf, File1.dataSize, frequency);

	// Attach buffer to source
	alSourcei(helloSource, AL_BUFFER, helloBuffer);

	// Properties that can affect the sound in 3D space

	ALfloat SourcePos[] = { 0.0,0.0,0.0 };
	ALfloat SourceVel[] = { 0.0,0.0,0.0 };
	ALfloat ListenerPos[] = { 0.0,0.0,-3.0 };
	ALfloat ListenerVel[] = { 0.0,0.0,0.0 };
	ALfloat ListenerOri[] = { 0.0,0.0,-1.0, 0.0, 1.0, 0.0 };

	// Set the proerties created above
	alListenerfv(AL_POSITION, ListenerPos);
	alListenerfv(AL_VELOCITY, ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);
	alSourcef(helloSource, AL_GAIN, 0.1);
	alSourcef(helloSource, AL_PITCH, 50);
	alSourcefv(helloSource, AL_POSITION, SourcePos);
	alSourcefv(helloSource, AL_VELOCITY, SourceVel);
	alSourcei(helloSource, AL_LOOPING, AL_TRUE);
///////////////end audio portion

    qhStart();
    return 0;
}



void button1DownCallback(unsigned int ShapeID)
{
	moveAll = false;

	if (ShapeID == 7)
	{
		moveAll = true;
	}
}

void button1UpCallback(unsigned int ShapeID)
{
	moveAll = false;
}

void button2DownCallback(unsigned int ShapeID)
{
	alSourcePlay(helloSource);
}

void button2UpCallback(unsigned int ShapeID)
{
	//reset rotation of the eye
	globalDragTransform = skull->getTransform();
	sclera->setTransform(globalDragTransform);
	pupil->setTransform(globalDragTransform);
	retina->setTransform(globalDragTransform);
	retina2->setTransform(globalDragTransform);
	iris->setTransform(globalDragTransform);
	nerve1->setTransform(globalDragTransform);
	nerve2->setTransform(globalDragTransform);
	nerve3->setTransform(globalDragTransform);
	rightIris->setTransform(globalDragTransform);
	rightSclera->setTransform(globalDragTransform);
	accumEyeRotation = 0;
	rotateEye = false;
	rotateEyeRight = false;
	////stop all the effects
	vibration = false;
	fulcrum_eff = false;
	infoBox1->setTexture("images/general_view.jpg");
	printf("effects disabled\n");
	OmniSpace->setDamping(0.0, 0.0);
	alSourcef(helloSource, AL_GAIN, 0.1);
	alSourcePause(helloSource);
}


/******************************************************************************
 Servo loop thread callback called when the effect is started.
******************************************************************************/
void HLCALLBACK startEffectCB(HLcache *cache, void *userdata)
{
	printf("Custom effect started\n");
}

/***************************************************************************************
 Servo loop thread callback.  Computes a force effect.
****************************************************************************************/
void HLCALLBACK computeForceCB(HDdouble force[3], HLcache *cache, void *userdata)
{
	hduVector3Dd vibrForce(0, 0, 0);
	hduVector3Dd fulcrumForce(0, 0, 0);



	if (save_Fulcrum_Point) {//get the first fulcrum point for reference
		hlCacheGetDoublev(cache, HL_PROXY_POSITION, fulcrumPoint);
		std::cout << "fulcrum/line point saved " << fulcrumPoint << std::endl;
		save_Fulcrum_Point = false;
	}

	//////fulcrum effect
	if (fulcrum_eff == true) {

		// Get haptic cursor position
		hlCacheGetDoublev(cache, HL_PROXY_POSITION, pos);
		// Get haptic cursor orientation
		hduQuaternion ori;
		hlCacheGetDoublev(cache, HL_PROXY_ROTATION, ori);
		// create unit vector along Z axis of stylus
		hduQuaternion unitVector(0, hduVector3Dd(0, 0, 1));
		// rotate unit vector to match haptic stylus
		unitVector = ori.conjugate() * unitVector* ori;
		// define two points: fulcrum and stylus tip 
		hduVector3Dd p0 = fulcrumPoint;
		hduVector3Dd p1 = fulcrumPoint + unitVector.v();
		hduLine <float> line(p0, p1);
		// desired haptic position is the projection of the current haptic position onto the fulcrum line 
		hduVector3Dd newPos = line.project(pos);
		// generate force between the current position and desired position 
		float gain = 0.5;
		hduVector3Dd newForce = (newPos - pos) * gain + vibrForce;
		// cap the force magnitude to maxForce
		float maxForce = 3.0;
		float length = (float)newForce.magnitude();
		if (length > maxForce)
		{
			newForce.normalize();
			newForce *= maxForce;
		}
		hduVector3Dd accumForce;
		hdGetDoublev(HD_CURRENT_FORCE, accumForce);
		hduVector3Dd totalForce = accumForce + newForce;
		// cap the force magnitude to maxForce 
		float maxTotalForce = 3.0;
		float forceLength = (float)totalForce.magnitude();
		if (forceLength > maxTotalForce)
		{
			totalForce.normalize();
			totalForce *= maxTotalForce;
		}

			//sclera->setRotationInPlace(fulcrumPoint, totalForce.magnitude());
			//pupil->setRotationInPlace(fulcrumPoint, totalForce.magnitude());
			//retina->setRotationInPlace(totalForce, totalForce.magnitude());
			//retina2->setRotationInPlace(totalForce, totalForce.magnitude());
			//iris->setRotationInPlace(totalForce, totalForce.magnitude());
			//nerve1->setRotationInPlace(totalForce, totalForce.magnitude());
			//nerve2->setRotationInPlace(totalForce, totalForce.magnitude());
			//nerve3->setRotationInPlace(totalForce, totalForce.magnitude());



		hdSetDoublev(HD_CURRENT_FORCE, totalForce);
		//std::cout << "accum rotate fulcrum" << accumEyeRotation << std::endl;
	}

///////////////////vibration

	if (vibration == true) {
		if (((count % 16) >= 0 && (count % 16) < 4) || ((count % 16) >= 8 && (count % 8) < 12))
			vibrForce = hduVector3Dd(0, 0, 0);
		else if ((count % 16) >= 4 && (count % 16) < 8)
			vibrForce = hduVector3Dd(0, 0, 1);
		else
			vibrForce = hduVector3Dd(0, 0, -1);
		count++;
		hdSetDoublev(HD_CURRENT_FORCE, vibrForce);
	}

	if (count >= 1024) { ///timer for the vibration while the warning is shown on the screen
		vibration = false;
		count = 0;
	}

}


/******************************************************************************
 Servo loop thread callback called when the effect is stopped.
******************************************************************************/
void HLCALLBACK stopEffectCB(HLcache *cache, void *userdata)
{
	printf("Custom effect stopped\n");
}


void graphicsCallback()
{
	//move all the objects at the same time
	if (moveAll == true) {
		globalDragTransform = skull->getTransform();
		sclera->setTransform(globalDragTransform);
		pupil->setTransform(globalDragTransform);
		retina->setTransform(globalDragTransform);
		retina2->setTransform(globalDragTransform);
		iris->setTransform(globalDragTransform);
		nerve1->setTransform(globalDragTransform);
		nerve2->setTransform(globalDragTransform);
		nerve3->setTransform(globalDragTransform);
		rightIris->setTransform(globalDragTransform);
		rightSclera->setTransform(globalDragTransform);
	}


	//rotates the eye
	if (rotateEye == true && accumEyeRotation < 25)
	{ 
		force = OmniSpace->getForce();
		accumEyeRotation = accumEyeRotation + (force.magnitude()*10);
		//rotates the angle and the direction of the force
		sclera->setRotationInPlace(force, force.magnitude() * 10);
		pupil->setRotationInPlace(force, force.magnitude() * 10);
		retina->setRotationInPlace(force, force.magnitude() * 10);
		retina2->setRotationInPlace(force, force.magnitude() * 10);
		iris->setRotationInPlace(force, force.magnitude() * 10);
		nerve1->setRotationInPlace(force, force.magnitude() * 10);
		nerve2->setRotationInPlace(force, force.magnitude() * 10);
		nerve3->setRotationInPlace(force, force.magnitude() * 10);
	}

	if (rotateEyeRight == true && accumEyeRotation < 15)
	{
		force = OmniSpace->getForce();
		accumEyeRotation = accumEyeRotation + (force.magnitude() * 3);
		//rotates the angle and the direction of the force
		rightIris->setRotationInPlace(force, force.magnitude() * 3);
		rightSclera->setRotationInPlace(force, force.magnitude() * 3);

	}




	//audio
	DisplayObject->getCamera(&FOV, &NearPlane, &FarPlane, &Eye, &LookAt, &UpVector); //get camera 
	ListenerPos[0] = Eye[0];
	ListenerPos[1] = Eye[1];
	ListenerPos[2] = Eye[2];
	//std::cout << "listener " << Eye << std::endl;
	alListenerfv(AL_POSITION, ListenerPos);

}


void motionCallback(unsigned int ShapeID)
{
	///audio
	cursorPosition = PlaygroundCursor->getPosition(); //get the position of the cursor
	nervePosition = nerve1->getCentroid()*nerve1->getTransform();
	distanceCursorNerve = magnitude(cursorPosition - nervePosition);
	SourcePos[0] = cursorPosition[0];
	SourcePos[1] = cursorPosition[1];
	SourcePos[2] = cursorPosition[2];
	alSourcefv(helloSource, AL_POSITION, SourcePos);
	alSourcef(helloSource, AL_GAIN, (1/distanceCursorNerve));


}


void touchCallback(unsigned int ShapeID)
{
	if (ShapeID == 8) { /////////if the lower layer of the retina is touched, warning about fibration
		vibration = true;
		std::cout << "You applied too much force to the retina. This amount of force can be dangerous to the tissue!" << std::endl; // console output
		txtWarning->setText("You applied too much force to the retina. This amount of force can be dangerous to the tissue!"); // warning text on the screen
		txtWarning->setGraphicVisibility(true);
	}
	if (ShapeID == 6) {
		std::cout << "Eye rotation enabled" << std::endl;
		rotateEye = true;
	}
	if (ShapeID == 14) {
		std::cout << "Eye Right rotation enabled" << std::endl;
		rotateEyeRight = true;
	}
	if (ShapeID == 5 || ShapeID == 10) {
		txtWarning->setText("Avoid touching the iris and the pupil during the incision. Follow the drawings to select the correct position!"); // warning text on the screen
		txtWarning->setGraphicVisibility(true);
	}
}


void unTouchCallback(unsigned int ShapeID)
{
	force = OmniSpace->getForce();
	std::cout << "Magnitude of the force just applied: " << force.magnitude() << std::endl; //show the magnitude of each touch action.
	//damping effect for the sclera and activation of the fulcrum effect
	if (ShapeID == 6 || ShapeID == 14)
	{
		fulcrum_eff = false;
		vibration = false;

		if ((force.magnitude() > 0.2))
		{
			infoBox1->setTexture("images/mp.jpg");
			fulcrum_eff = true; //starts the fulcrum effect
			save_Fulcrum_Point = true;//use this flag to save the fulcrum point for the fulcrum and line effect
			std::cout << "You reached the vitreous chamber of the eye" << std::endl; // console output
			txtWarning->setText("You reached the vitreous chamber, move gently"); // warning text on the screen
			txtWarning->setGraphicVisibility(true);
			OmniSpace->setDamping(0.5, 0.5); // gain and magnitude of damping
			//sclera->setTouchableFace("Back");// alternate the touchable face for the return of the tool
			//rightSclera->setTouchableFace("Back");// alternate the touchable face for the return of the tool
		}
	}
	accumEyeRotation = 0;
	rotateEye = false;
	rotateEyeRight = false;
	globalDragTransform = skull->getTransform();
	sclera->setTransform(globalDragTransform);
	pupil->setTransform(globalDragTransform);
	retina->setTransform(globalDragTransform);
	retina2->setTransform(globalDragTransform);
	iris->setTransform(globalDragTransform);
	nerve1->setTransform(globalDragTransform);
	nerve2->setTransform(globalDragTransform);
	nerve3->setTransform(globalDragTransform);
	rightSclera->setTransform(globalDragTransform);
	rightIris->setTransform(globalDragTransform);
	//sclera->setTouchableFace("Front");// alternate the touchable face for the return of the tool
	//rightSclera->setTouchableFace("Front");// alternate the touchable face for the return of the tool

}