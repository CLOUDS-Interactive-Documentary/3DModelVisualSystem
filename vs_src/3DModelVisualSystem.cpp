//
//  CloudsVisualSystem3DModel.cpp
//

#include "3DModelVisualSystem.h"
#include "CloudsRGBDVideoPlayer.h"

//#include "CloudsRGBDVideoPlayer.h"
//#ifdef AVF_PLAYER
//#include "ofxAVFVideoPlayer.h"
//#endif

//These methods let us add custom GUI parameters and respond to their events
void CloudsVisualSystem3DModel::selfSetupGui(){

	customGui = new ofxUISuperCanvas("CUSTOM", gui);
	customGui->copyCanvasStyle(gui);
	customGui->copyCanvasProperties(gui);
	customGui->setName("Custom");
	customGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
	customGui->addFPS();
	
//	customGui->addSlider("Custom Float 1", 1, 1000, &customFloat1);
//	customGui->addSlider("Custom Float 2", 1, 1000, &customFloat2);
//	customGui->addButton("Custom Button", false);
//	customGui->addToggle("Custom Toggle", &customToggle);
	
	ofAddListener(customGui->newGUIEvent, this, &CloudsVisualSystem3DModel::selfGuiEvent);
	guis.push_back(customGui);
	guimap[customGui->getName()] = customGui;
}

void CloudsVisualSystem3DModel::selfGuiEvent(ofxUIEventArgs &e){
//	if(e.widget->getName() == "Custom Button"){
//		cout << "Button pressed!" << endl;
//	}
}

//Use system gui for global or logical settings, for exmpl
void CloudsVisualSystem3DModel::selfSetupSystemGui(){
	
}

void CloudsVisualSystem3DModel::guiSystemEvent(ofxUIEventArgs &e){
	
}
//use render gui for display settings, like changing colors
void CloudsVisualSystem3DModel::selfSetupRenderGui(){

}

void CloudsVisualSystem3DModel::guiRenderEvent(ofxUIEventArgs &e){
	
}

// selfSetup is called when the visual system is first instantiated
// This will be called during a "loading" screen, so any big images or
// geometry should be loaded here
void CloudsVisualSystem3DModel::selfSetup(){
	
	videoLoaded = false;
	
//	if(ofFile::doesFileExist(getVisualSystemDataPath() + "TestVideo/Jer_TestVideo.mov")){
//		getRGBDVideoPlayer().setup(getVisualSystemDataPath() + "TestVideo/Jer_TestVideo.mov",
//								   getVisualSystemDataPath() + "TestVideo/Jer_TestVideo.xml" );
//		
//		getRGBDVideoPlayer().swapAndPlay();
//		
//		for(int i = 0; i < 640; i += 2){
//			for(int j = 0; j < 480; j+=2){
//				simplePointcloud.addVertex(ofVec3f(i,j,0));
//			}
//		}
//		
//		pointcloudShader.load(getVisualSystemDataPath() + "shaders/rgbdcombined");
//		videoLoaded = true;
//	}
//	
	
//	someImage.loadImage( getVisualSystemDataPath() + "images/someImage.png";
	
	//set defaults
	gridLineWidth = 1.;
	boundBoxLineWidth = 1.;
	discardThreshold = 1.;
	bSmoothModel = false;
	bComputeSmoothNormals = false;
	
	//load our shaders
	loadShaders();
	
	//get list of models from the model directory
	modelScl.set( 1,1,1 );
	loadModel("models/elephant.obj");
	
	//setup a grid vbo
	float gridDim = 100;
	float halfGridDim = gridDim / 2;
	vector<ofVec3f> gridVertices(gridDim * 4);
	for (int i=0; i<gridDim; i++) {
		gridVertices[i*4+0].set(i - halfGridDim, 0,-halfGridDim);
		gridVertices[i*4+1].set(i - halfGridDim, 0, halfGridDim);
		gridVertices[i*4+2].set(-halfGridDim, 0, i - halfGridDim);
		gridVertices[i*4+3].set( halfGridDim, 0, i - halfGridDim);
	}
	grid.setVertexData( &gridVertices[0], gridVertices.size(), GL_STATIC_DRAW );
	numGridVertices = gridVertices.size();
	gridVertices.clear();
	
	//setup boundBox vbo
	calcBoundingBox();
	vector<ofVec3f> bbVerts(24);
	bbVerts[0].set(-.5,-.5,-.5);
	bbVerts[1].set( .5,-.5,-.5);
	bbVerts[2].set(-.5, .5,-.5);
	bbVerts[3].set( .5, .5,-.5);
	bbVerts[4].set(-.5,-.5,-.5);
	bbVerts[5].set(-.5, .5,-.5);
	bbVerts[6].set( .5,-.5,-.5);
	bbVerts[7].set( .5, .5,-.5);
	bbVerts[8].set(-.5,-.5, .5);
	bbVerts[9].set( .5,-.5, .5);
	bbVerts[10].set(-.5, .5, .5);
	bbVerts[11].set( .5, .5, .5);
	bbVerts[12].set(-.5,-.5, .5);
	bbVerts[13].set(-.5, .5, .5);
	bbVerts[14].set( .5,-.5, .5);
	bbVerts[15].set( .5, .5, .5);
	bbVerts[16].set( .5, .5,-.5);
	bbVerts[17].set( .5, .5, .5);
	bbVerts[18].set(-.5, .5,-.5);
	bbVerts[19].set(-.5, .5, .5);
	bbVerts[20].set(-.5,-.5,-.5);
	bbVerts[21].set(-.5,-.5, .5);
	bbVerts[22].set( .5,-.5,-.5);
	bbVerts[23].set( .5,-.5, .5);
	
	boundBoxVbo.setVertexData( &bbVerts[0], bbVerts.size(), GL_STATIC_DRAW );
	bbVerts.clear();
	
}

// selfPresetLoaded is called whenever a new preset is triggered
// it'll be called right before selfBegin() and you may wish to
// refresh anything that a preset may offset, such as stored colors or particles
void CloudsVisualSystem3DModel::selfPresetLoaded(string presetPath){
	
}

// selfBegin is called when the system is ready to be shown
// this is a good time to prepare for transitions
// but try to keep it light weight as to not cause stuttering
void CloudsVisualSystem3DModel::selfBegin(){
	
}

//do things like ofRotate/ofTranslate here
//any type of transformation that doesn't have to do with the camera
void CloudsVisualSystem3DModel::selfSceneTransformation(){
	
}

//normal update call
void CloudsVisualSystem3DModel::selfUpdate(){

	cout << "WTF" << endl;
	
	
}

// selfDraw draws in 3D using the default ofEasyCamera
// you can change the camera by returning getCameraRef()
void CloudsVisualSystem3DModel::selfDraw(){
	
//	if(videoLoaded){
//		ofPushMatrix();
//		setupRGBDTransforms();
//		pointcloudShader.begin();
//		getRGBDVideoPlayer().setupProjectionUniforms(pointcloudShader);
//		simplePointcloud.drawVertices();
//		pointcloudShader.end();
//		ofPopMatrix();
//	}
	
	

	
	//update the model transforms
	modelRot.makeRotate( ofGetElapsedTimef()*10, 0, 1, 0);
	if(modelScl.length() == 0.)	modelScl.y = .00001;
	
	modelTransform.setPosition( modelPos + ofVec3f(0,maxBound.y,0) );
	modelTransform.setOrientation( modelRot );
	modelTransform.setScale( modelScl );
	
	
	//draw our model
	ofPushMatrix();
	ofMultMatrix( modelTransform.getGlobalTransformMatrix() );
	
	normalShader.begin();
//	normalShader.setUniform1f( "discardThreshold", discardThreshold );
//	modelMesh.draw();
	
	smoothedMesh.draw();
	
	normalShader.end();
	
	
	ofSetColor(255, 255, 255, 255);
	drawBoundingBox();
	
	ofPopMatrix();
	
	
	//draw grid
	glLineWidth( gridLineWidth );
	
	ofSetColor(255,255, 255, 100 );
	
	ofPushMatrix();
	ofScale( 25,25,25 );
	
	gridShader.begin();
	grid.draw(GL_LINES, 0, numGridVertices );
	gridShader.end();
	
	ofPopMatrix();
	
}

// draw any debug stuff here
void CloudsVisualSystem3DModel::selfDrawDebug(){
	
}
// or you can use selfDrawBackground to do 2D drawings that don't use the 3D camera
void CloudsVisualSystem3DModel::selfDrawBackground(){

	//turn the background refresh off
	//bClearBackground = false;
	
}
// this is called when your system is no longer drawing.
// Right after this selfUpdate() and selfDraw() won't be called any more
void CloudsVisualSystem3DModel::selfEnd()
{
	
	simplePointcloud.clear();
	
}
// this is called when you should clear all the memory and delet anything you made in setup
void CloudsVisualSystem3DModel::selfExit()
{
	//???: these should be here and not in selfEnd() right?
	boundBoxVbo.clear();
	modelMesh.clear();
	grid.clear();
}

//events are called when the system is active
//Feel free to make things interactive for you, and for the user!
void CloudsVisualSystem3DModel::selfKeyPressed(ofKeyEventArgs & args){
	if( args.key == 'l' || args.key == 'L' ){
		loadShaders();
	}
}
void CloudsVisualSystem3DModel::selfKeyReleased(ofKeyEventArgs & args){
	
}

void CloudsVisualSystem3DModel::selfMouseDragged(ofMouseEventArgs& data){
	
}

void CloudsVisualSystem3DModel::selfMouseMoved(ofMouseEventArgs& data){
	
}

void CloudsVisualSystem3DModel::selfMousePressed(ofMouseEventArgs& data){
	
}

void CloudsVisualSystem3DModel::selfMouseReleased(ofMouseEventArgs& data){
	
}

void CloudsVisualSystem3DModel::loadShaders(){
	normalShader.load( getVisualSystemDataPath() + "shaders/normalShader" );
	gridShader.load( getVisualSystemDataPath() + "shaders/gridShader" );
}

void CloudsVisualSystem3DModel::calcBoundingBox(){
	minBound.set(10000000,10000000,10000000), maxBound.set(-10000000,-10000000,-10000000);
	
	vector<ofVec3f>& v = modelMesh.getVertices();
	for(int i=0; i<v.size(); i++){
		minBound.x = min(minBound.x, v[i].x);
		minBound.y = min(minBound.y, v[i].y);
		minBound.z = min(minBound.z, v[i].z);
		
		maxBound.x = max(maxBound.x, v[i].x);
		maxBound.y = max(maxBound.y, v[i].y);
		maxBound.z = max(maxBound.z, v[i].z);
	}
	
	//cout << minBound << " : " << maxBound << endl;
	boundCenter = ( minBound + maxBound ) * .5;
};


void CloudsVisualSystem3DModel::drawBoundingBox(){
	
	glLineWidth( boundBoxLineWidth );
	
	ofPushMatrix();
	ofTranslate(boundCenter);
	ofScale(maxBound.x - minBound.x, maxBound.y - minBound.y, maxBound.z - minBound.z);

	boundBoxVbo.draw(GL_LINES, 0, 24);
	
	ofPopMatrix();
};


void CloudsVisualSystem3DModel::loadModel( string fileName, bool smoothing )
{
	ofxObjLoader::load( getVisualSystemDataPath() + fileName, modelMesh, true);
	
	smoothVertices( modelMesh, smoothedMesh );
}

string CloudsVisualSystem3DModel::vec3ToString( ofVec3f v, int precision ){
	string outstr;
	
	outstr += ofToString( v.x, precision) + "_" + ofToString( v.y, precision) + "_" + ofToString( v.z, precision);
	
	return outstr;
}

ofVec3f CloudsVisualSystem3DModel::normalFrom3Points(ofVec3f p0, ofVec3f p1, ofVec3f p2)
{
	ofVec3f norm = (p2 - p1).cross( p0 - p1);
	return norm.normalized();
}

void CloudsVisualSystem3DModel::smoothVertices( ofMesh& facetedMesh, ofMesh& targetMesh, int precision)
{
	map<string, unsigned int> mergeMap;
	vector<ofVec3f>& v = facetedMesh.getVertices();
	vector<ofVec2f>& uv = facetedMesh.getTexCoords();
	vector<ofIndexType>& indices = facetedMesh.getIndices();
	
	vector<ofVec3f> smoothVertices;
	vector<ofVec3f> smoothNormals;
	vector<ofVec2f> smoothTexCoords;
	vector<ofIndexType> smoothIndices;
	
	//change face indices to match
	string vStr;
	for (int i=0; i<v.size(); i++)
	{
		vStr = vec3ToString( v[i], precision );
		mergeMap[ vStr ] = i;
	}
	
	//fill our smoothed vertex array with merged vertices
	smoothVertices.resize( mergeMap.size() );
	smoothTexCoords.resize( mergeMap.size() );
	int svCount = 0;
	for (map<string, unsigned int>::iterator it = mergeMap.begin(); it != mergeMap.end(); it++)
	{
		smoothVertices[svCount] = v[it->second];
		smoothTexCoords[svCount] = uv[it->second];
		it->second = svCount;//store our new vertex index
		svCount++;
	}
	
	
	//reconstruct our faces by reassigning ther indices
	smoothIndices.resize( indices.size() );
	for (int i=0; i<indices.size(); i++)
	{
		//use our old vertex poisition to retrieve our new index
		smoothIndices[i] = mergeMap[ vec3ToString( v[ indices[i] ], precision ) ];
	}
	
	//calculate our normals
	smoothNormals.resize( smoothVertices.size() );
	ofVec3f n;
	for (int i=0; i<smoothIndices.size(); i+=3)
	{
		n = normalFrom3Points( smoothVertices[smoothIndices[i]],smoothVertices[smoothIndices[i+1]], smoothVertices[smoothIndices[i+2]] );
		smoothNormals[smoothIndices[i]] += n;
		smoothNormals[smoothIndices[i+1]] += n;
		smoothNormals[smoothIndices[i+2]] += n;
	}
	
	for (int i=0; i<smoothNormals.size(); i++)
	{
		smoothNormals[i].normalize();
	}
	
	
	//setup our smoothed mesh
	//TODO: texture coords!
	targetMesh.addVertices( smoothVertices );
	targetMesh.addNormals( smoothNormals );
	targetMesh.addTexCoords(smoothTexCoords );
	targetMesh.addIndices( smoothIndices );
	
}

