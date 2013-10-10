//
//  CloudsOrthoCamera.cpp
//  3DModelVisualSystem
//
//  Created by laserstorm on 10/7/13.
//
//

#include "CloudsOrthoCamera.h"


#include "ofEasyCam.h"
#include "ofMath.h"
#include "ofUtils.h"

// when an ofEasyCam is moving due to momentum, this keeps it
// from moving forever by assuming small values are zero.
float minDifference = 0.1e-5;

// this is the default on windows os
unsigned long doubleclickTime = 200;

//----------------------------------------
CloudsOrthoCamera::CloudsOrthoCamera(){
	lastTap	= 0;
	lastDistance = 0;
	drag = 0.9f;
	sensitivityRot = 1.0f;//when 1 moving the mouse from one side to the other of the arcball (min(viewport.width, viewport.height)) will rotate 180degrees. when .5, 90 degrees.
	sensitivityXY = .5;
	sensitivityZ= .7;
	
	bDistanceSet = false;
	bMouseInputEnabled = false;
	bDoRotate = false;
	bApplyInertia =false;
	bDoTranslate = false;
	bInsideArcball = true;
	bValidClick = false;
	bEnableMouseMiddleButton = true;
	bAutoDistance = true;
	doTranslationKey = 'm';
	
	reset();
	enableMouseInput();
	
	orthoViewScale = 1.;
	
	bDisableEasyCamControls = false;
	bOrbitMode = true;
	bExploreMode = false;
}

//----------------------------------------
CloudsOrthoCamera::~CloudsOrthoCamera(){
	disableMouseInput();
}
//----------------------------------------
void CloudsOrthoCamera::update(ofEventArgs & args){
    if(!bDistanceSet && bAutoDistance){
        setDistance(getImagePlaneDistance(viewport), true);
    }
	
    if(bDisableEasyCamControls && bMouseInputEnabled){
		
		rotationFactor = sensitivityRot * 180 / min(viewport.width, viewport.height);
		if (bMouseInputEnabled) {
			updateMouse();
		}
		
		if (bDoRotate) {
			updateRotation();
		}else if (bDoTranslate) {
			updateTranslation();
		}
	}
	
	
	if( bExploreMode && !ofGetMousePressed() && viewport.inside( ofGetMouseX(), ofGetMouseY() ) )
	{
		float mouseScl = .5;
		float moveZone = .1;
		float cameraSpeed = 1.;
		float pitchScale = .005;
		
		//convert mouse coords in to somethin we can work with
		float mx = ofMap( ofGetMouseX(), viewport.getLeft(), viewport.getRight(), 1., -1., true );
		float my = ofMap( ofGetMouseY(), viewport.getTop(), viewport.getBottom(), 1., -1., true );
		float dist = ofVec2f(mx, my).length();
		
		if(dist > moveZone)
		{
			float weight = ofMap( dist - moveZone, 0, 1, 0, 1, true );
			
			//dead zone in the middle where nowe just sit an stare
			mx *= weight;
			my *= weight;
			
			float panVal = mx * mouseScl;
			float tiltVal = my * mouseScl;
			
			//rotate our camera accordingly
			pan( panVal );
			tilt( tiltVal );
			roll( getPitch() * -pitchScale );
		}
		
		ofVec3f vel = getLookAtDir();
		move( vel * ofClamp(1. - dist, 0, 1) * cameraSpeed );
		
		target.setPosition( getPosition() + vel * 10. );
	}
	
	if(bOrbitMode && viewport.inside( ofGetMouseX(), ofGetMouseY() ) )
	{
		//TODO: make getters and setters for these attributes
		float mouseScl = .5;
		float deadZone = .1;
		float cameraSpeed = 1.;
		float pitchScale = .01;
		
		//convert mouse coords in to somethin we can work with
		float mx = ofMap( ofGetMouseX(), viewport.getLeft(), viewport.getRight(), -1., 1., true );
		float my = ofMap( ofGetMouseY(), viewport.getTop(), viewport.getBottom(), -1., 1., true );
		float dist = ofVec2f(mx, my).length();
		
		if(dist > deadZone)
		{
			//the deadzone is an area in the center of the screen where we don't rotate
			float weight = ofMap( dist - deadZone, 0, 1, 0, 1, true );
			
			//so that we don't rotate past verticle we'll scale down our rotation as it approaches 90 degrees
			float xScl = ofMap( abs(getRoll()), 60, 89, 1, 0, true );
			
			//get our rotation values and update the rotation aroundd the target
			xRot = my * weight * mouseScl * xScl;
			yRot = mx * weight * mouseScl;
			zRot = 0;
			updateRotation();
			
			//auto level
			roll( getPitch() * -pitchScale ); // it seems like easy cams get pitch and get roll are reversed...
		}
	}
}
//----------------------------------------
void CloudsOrthoCamera::begin(ofRectangle viewport ){
	this->viewport = viewport;
//	ofCamera::begin(viewport);
	
	if(!isActive) ofPushView();
	isActive = true;

	ofSetCoordHandedness(OF_RIGHT_HANDED);

	// autocalculate near/far clip planes if not set by user
	calcClipPlanes(viewport);

	ofSetMatrixMode(OF_MATRIX_PROJECTION);
	ofLoadIdentityMatrix();

	ofLoadMatrix( this->getProjectionMatrix(viewport) );

	ofSetMatrixMode(OF_MATRIX_MODELVIEW);
	ofLoadMatrix( getModelViewMatrix( viewport ) );// ofMatrix4x4::getInverseOf(getGlobalTransformMatrix()) );
	ofViewport(viewport);
}

//----------------------------------------
ofMatrix4x4 CloudsOrthoCamera::getProjectionMatrix(ofRectangle viewport) const {
	if(isOrtho) {
		ofMatrix4x4 ortho;
		ortho.makeOrthoMatrix(0, viewport.width * orthoViewScale, 0, viewport.height * orthoViewScale, -farClip * orthoViewScale, farClip * orthoViewScale);
		return ortho;
	}else{
		float aspect = forceAspectRatio ? aspectRatio : viewport.width/viewport.height;
		ofMatrix4x4 matProjection;
		matProjection.makePerspectiveMatrix(fov, aspect, nearClip, farClip);
		matProjection.translate(-lensOffset.x, -lensOffset.y, 0);
		return matProjection;
	}
}
//----------------------------------------
ofMatrix4x4 CloudsOrthoCamera::getModelViewMatrix(ofRectangle viewport) const {
	ofMatrix4x4 matModelView;
	matModelView.makeInvertOf( getGlobalTransformMatrix() );

	if(getOrtho())	matModelView.translate( orthoViewScale * viewport.getWidth()/2, orthoViewScale * viewport.getHeight()/2, 0);
	
	return matModelView;
}

//----------------------------------------
void CloudsOrthoCamera::reset(){
	target.resetTransform();
	
	target.setPosition(0,0, 0);
	lookAt(target);
	
	resetTransform();
	setPosition(0, 0, lastDistance);
	
	
	xRot = 0;
	yRot = 0;
	zRot = 0;
	
	moveX = 0;
	moveY = 0;
	moveZ = 0;
}
//----------------------------------------
void CloudsOrthoCamera::setTarget(const ofVec3f& targetPoint){
	target.setPosition(targetPoint);
	lookAt(target);
}
//----------------------------------------
void CloudsOrthoCamera::setTarget(ofNode& targetNode){
	target = targetNode;
	lookAt(target);
}
//----------------------------------------
ofNode& CloudsOrthoCamera::getTarget(){
	return target;
}
//----------------------------------------
void CloudsOrthoCamera::setDistance(float distance){
	setDistance(distance, true);
}
//----------------------------------------
void CloudsOrthoCamera::setDistance(float distance, bool save){//should this be the distance from the camera to the target?
	if (distance > 0.0f){
		if(save){
			this->lastDistance = distance;
		}
		setPosition(target.getPosition() + (distance * getZAxis()));
		bDistanceSet = true;
	}
}
//----------------------------------------
float CloudsOrthoCamera::getDistance() const {
	return target.getPosition().distance(getPosition());
}
//----------------------------------------
void CloudsOrthoCamera::setAutoDistance(bool bAutoDistance){
    this->bAutoDistance = bAutoDistance;
    if (bAutoDistance) {
        bDistanceSet = false;
    }
}
//----------------------------------------
void CloudsOrthoCamera::setDrag(float drag){
	this->drag = drag;
}
//----------------------------------------
float CloudsOrthoCamera::getDrag() const {
	return drag;
}
//----------------------------------------
void CloudsOrthoCamera::setTranslationKey(char key){
	doTranslationKey = key;
}
//----------------------------------------
char CloudsOrthoCamera::getTranslationKey(){
	return doTranslationKey;
}
//----------------------------------------
void CloudsOrthoCamera::enableMouseInput(){
	if(!bMouseInputEnabled){
		bMouseInputEnabled = true;
		//	ofRegisterMouseEvents(this);
		ofAddListener(ofEvents().update , this, &CloudsOrthoCamera::update);
	}
}
//----------------------------------------
void CloudsOrthoCamera::disableMouseInput(){
	if(bMouseInputEnabled){
		bMouseInputEnabled = false;
		//ofUnregisterMouseEvents(this);
		ofRemoveListener(ofEvents().update, this, &CloudsOrthoCamera::update);
	}
	setDrag( 0 );
}
//----------------------------------------
bool CloudsOrthoCamera::getMouseInputEnabled(){
	return bMouseInputEnabled;
}
//----------------------------------------
void CloudsOrthoCamera::enableMouseMiddleButton(){
	bEnableMouseMiddleButton = true;
}
//----------------------------------------
void CloudsOrthoCamera::disableMouseMiddleButton(){
	bEnableMouseMiddleButton = false;
}
//----------------------------------------
bool CloudsOrthoCamera::getMouseMiddleButtonEnabled(){
	return bEnableMouseMiddleButton;
}
//----------------------------------------
void CloudsOrthoCamera::updateTranslation(){
	if (bApplyInertia) {
		moveX *= drag;
		moveY *= drag;
		moveZ *= drag;
		if (ABS(moveX) <= minDifference && ABS(moveY) <= minDifference && ABS(moveZ) <= minDifference) {
			bApplyInertia = false;
			bDoTranslate = false;
		}
	}
	move((getXAxis() * moveX) + (getYAxis() * moveY) + (getZAxis() * moveZ));
	
	
	//ortho zoom
	if( getOrtho() )	orthoViewScale = ofClamp( orthoViewScale + moveZ * .01, .1, 100 );
}
//----------------------------------------
void CloudsOrthoCamera::updateRotation(){
	if (bApplyInertia) {
		xRot *=drag;
		yRot *=drag;
		zRot *=drag;
		
		if (ABS(xRot) <= minDifference && ABS(yRot) <= minDifference && ABS(zRot) <= minDifference) {
			bApplyInertia = false;
			bDoRotate = false;
		}
	}
	curRot = ofQuaternion(xRot, ofCamera::getXAxis(), yRot, ofCamera::getYAxis(), zRot, ofCamera::getZAxis());
	setPosition((ofCamera::getGlobalPosition()-target.getGlobalPosition())*curRot +target.getGlobalPosition());
	rotate(curRot);
}
//----------------------------------------
void CloudsOrthoCamera::updateMouse(){
	mouse = ofVec2f(ofGetMouseX(), ofGetMouseY());
	if(viewport.inside(mouse.x, mouse.y) && !bValidClick && ofGetMousePressed()){
		unsigned long curTap = ofGetElapsedTimeMillis();
		if(lastTap != 0 && curTap - lastTap < doubleclickTime){
			reset();
		}
		if ((bEnableMouseMiddleButton && ofGetMousePressed(1)) || ofGetKeyPressed(doTranslationKey)  || ofGetMousePressed(2)){
			bDoTranslate = true;
			bDoRotate = false;
			bApplyInertia = false;
		}else if (ofGetMousePressed(0)) {
			bDoTranslate = false;
			bDoRotate = true;
			bApplyInertia = false;
			if(ofVec2f(mouse.x - viewport.x - (viewport.width/2), mouse.y - viewport.y - (viewport.height/2)).length() < min(viewport.width/2, viewport.height/2)){
				bInsideArcball = true;
			}else {
				bInsideArcball = false;
			}
		}
		lastTap = curTap;
		//lastMouse = ofVec2f(ofGetPreviousMouseX(),ofGetPreviousMouseY()); //this was causing the camera to have a tiny "random" rotation when clicked.
		lastMouse = mouse;
		bValidClick = true;
		bApplyInertia = false;
	}
	
	if (bValidClick) {
		if (!ofGetMousePressed()) {
			bApplyInertia = true;
			bValidClick = false;
		}else {
			mouseVel = mouse  - lastMouse;
			
			if (bDoTranslate) {
				moveX = 0;
				moveY = 0;
				moveZ = 0;
				if (ofGetMousePressed(2)) {
					moveZ = mouseVel.y * sensitivityZ * (getDistance() + FLT_EPSILON)/ viewport.height;
				}else {
					moveX = -mouseVel.x * sensitivityXY * (getDistance() + FLT_EPSILON)/viewport.width;
					moveY =  mouseVel.y * sensitivityXY * (getDistance() + FLT_EPSILON)/viewport.height;
				}
			}else {
				xRot = 0;
				yRot = 0;
				zRot = 0;
				if (bInsideArcball) {
					xRot = -mouseVel.y * rotationFactor;
					yRot = -mouseVel.x * rotationFactor;
				}else {
					ofVec2f center(viewport.width/2, viewport.height/2);
					zRot = - ofVec2f(mouse.x - viewport.x - center.x, mouse.y -viewport.y -center.y).angle(lastMouse - ofVec2f(viewport.x, viewport.y) - center);
				}
			}
			lastMouse = mouse;
		}
	}
}
