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
	
	customGui->addSpacer();
	
	customGui->addToggle("smooth model", false );
	customGui->addToggle("wireframe", &bWireframe );
	customGui->addSlider("wireframeLinewidth", 0.5, 10, &wireframeLinewidth);
	customGui->addSlider("discardThreshold", 0., 1, &discardThreshold);
	customGui->addSlider("modelScale", .1, 10., &modelScale);
	customGui->addImageSampler("c1", &colorMap, (float)colorMap.getWidth()/2, (float)colorMap.getHeight()/2 );
	
	customGui->addSlider("specularExpo", 1, 128, &specularExpo);
	customGui->addSlider("specularScale", 0., 1.0, &specularScale);
	
	customGui->addSpacer();
	
	customGui->addSlider("gridLineWidth", 0.5, 10, &gridLineWidth);
	customGui->addSlider("boundBoxLineWidth", 0.5, 10, &boundBoxLineWidth);
	customGui->addSlider("majorGridLineWidth", 0.5, 10, &majorGridLineWidth);
	customGui->addSlider("gridScale", 1., 100., &gridScale);
	
	customGui->addSpacer();
	
	customGui->addLabel("obj files:");
	customGui->addRadio("model files", objFiles );
	
	customGui->addSpacer();
	
	customGui->addRadio("shaders", shaderNames );
	
	ofAddListener(customGui->newGUIEvent, this, &CloudsVisualSystem3DModel::selfGuiEvent);
	guis.push_back(customGui);
	guimap[customGui->getName()] = customGui;
	
	
	modelUIGui = new ofxUISuperCanvas("modelUIGui", gui);
	modelUIGui->copyCanvasStyle(gui);
	modelUIGui->copyCanvasProperties(gui);
	modelUIGui->setName("modelUIGui");
	modelUIGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
	modelUIGui->addSpacer();
	modelUIGui->addLabel("arrows");
	modelUIGui->addSlider("arrowRadius", 1, 50, &arrowRadius);
	modelUIGui->addSlider("arrowHeight", 1, 250, &arrowHeight);
	modelUIGui->addSlider("arrowPointHeight", 0, .9, &arrowPointHeight);
	
	
	ofAddListener(modelUIGui->newGUIEvent, this, &CloudsVisualSystem3DModel::selfGuiEvent);
	guis.push_back(modelUIGui);
	guimap[modelUIGui->getName()] = modelUIGui;

	
}

void CloudsVisualSystem3DModel::selfGuiEvent(ofxUIEventArgs &e)
{
	string name = e.widget->getName();
	int kind = e.widget->getKind();
	
	if( name == "smooth model" )
	{
		bSmoothModel = e.getToggle()->getValue();
		if( bSmoothModel ){
			smoothMesh( modelMesh, modelMesh );
		}else{
			facetMesh( modelMesh, modelMesh );
		}
	}
	
	else if(name == "arrowHeight" || name == "arrowRadius" || name == "arrowPointHeight" )
	{
		resizeTheArrowMesh( arrowRadius, arrowHeight, arrowPointHeight );
	}
	
	else if( e.widget->getName() == "c1"){
		ofxUIImageSampler* sampler = (ofxUIImageSampler *) e.widget;
		modelColor = sampler->getColor();
	}
	
	else if( kind == OFX_UI_WIDGET_TOGGLE)
	{
		string parent = e.getToggle()->getParent()->getName();
		
		if( e.getToggle()->getValue() )
		{
			
			if (parent == "shaders")
			{
				//switch to this active shader from a shader map
				if(shaderMap.find( name ) != shaderMap.end() )
				{
					cout << parent << " : " << name << " : " << shaderMap[ name ] << endl;
					activeShader = shaderMap[ name ];
				}
			}
			
			else if(parent == "model files")
			{
				//load the model from the selected file
				for (int i=0; i<objFiles.size(); i++)
				{
					if(objFiles[i] == name )
					{
						cout << "loading model: " << name << endl;
						loadModel( "models/" + name, bSmoothModel );
					}
				}
			}
		}
	}
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
	
	//set defaults
	gridLineWidth = 1.;
	boundBoxLineWidth = 1.;
	discardThreshold = 1.;
	bSmoothModel = false;
	maxDim = 200;
	modelScl.set( 1,1,1 );
	gridScale = 25.;
	modelScale = 1.;
	modelColor.set(10,10,11);
	specularExpo = 32;
	specularScale = .75;
	majorGridLineWidth = 1.5;
	bWireframe = false;
	wireframeLinewidth = .5;
	activeShader = NULL;
	
	arrowRadius = 25;
	arrowHeight = 100;
	arrowPointHeight = .75;
	
	
	colorMap.loadImage( getVisualSystemDataPath() + "GUI/defaultColorPalette.png" );
	
	
	//load our shaders
	loadShaders();
	
	//load our non-model meshes
	ofxObjLoader::load("arrow.obj", arrowMesh, true );
	resizeTheArrowMesh( arrowRadius, arrowHeight, arrowPointHeight );
	
	loadCameraLineModel( cameraLines, "cameraVertices.txt" );
	
	//get list of models from the model directory
	string path = getVisualSystemDataPath() + "models/";
	ofDirectory dir;
	dir.allowExt("obj");
	dir.listDir( path );
	for(int i = 0; i < dir.numFiles(); i++){
		objFiles.push_back( dir.getName(i) );
	}
	
	//setup a grid vbos
	setupGridVbos();
	
	//setup boundBox vbo
	setupBoundingBoxVbo();
	calcBoundingBox();
	
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
void CloudsVisualSystem3DModel::selfDraw()
{
	
	//update the model transforms
	modelRot.makeRotate( ofGetElapsedTimef()*2, 0, 1, 0);
	if(modelScl.length() == 0.)	modelScl.y = .00001;
	
	modelTransform.setPosition( modelPos );//+ ofVec3f(0,min(maxBound.y,minBound.y),0) );
	modelTransform.move(0, -minBound.y * modelScl.y * modelScale, 0);
	modelTransform.setOrientation( modelRot );
	modelTransform.setScale( modelScl * modelScale );
	
	
	
	//draw our model
	ofPushMatrix();
	ofMultMatrix( modelTransform.getGlobalTransformMatrix() );
	
	ofSetColor(modelColor);
	if(activeShader != NULL )
	{
		
		activeShader->begin();
		activeShader->setUniform1f( "discardThreshold", discardThreshold );
		activeShader->setUniform1f( "specularExpo", specularExpo );
		activeShader->setUniform1f( "specularScale", specularScale );
//		uniform float specularScale = 1.;
//		uniform float specularExpo = 64.;
//		uniform float discardThreshold = .35;
		
		if(bWireframe)	glLineWidth( wireframeLinewidth );
		bWireframe?	modelMesh.drawWireframe() : modelMesh.draw();
		
		activeShader->end();
	}
	
	
	//draw bounding box
	ofSetColor(255, 255, 255, 255);
	drawBoundingBox();
	
	ofPopMatrix();
	
	
	//draw arrows at model's min bound
	ofPushMatrix();
	ofTranslate( minBound * modelTransform.getGlobalTransformMatrix() );
	ofMultMatrix( modelTransform.getGlobalOrientation() );
	
	ofSetColor(0, 255, 0);
	ofPushMatrix();
	ofScale( arrowScale.x, arrowScale.y, arrowScale.z );
	arrowMesh.draw();
	ofPopMatrix();
	
	ofSetColor(255, 0, 0);
	ofPushMatrix();
	ofRotate(90, 1, 0, 0);
	ofScale( arrowScale.x, arrowScale.y, arrowScale.z );
	arrowMesh.draw();
	ofPopMatrix();
	
	ofSetColor(0, 0, 255);
	ofPushMatrix();
	ofRotate(90, 1, 0, 0);
	ofRotate(90, 0, 0, -1);
	ofScale( arrowScale.x, arrowScale.y, arrowScale.z );
	arrowMesh.draw();
	ofPopMatrix();
	
	ofPopMatrix();
	
	
	//draw grid
	glLineWidth( gridLineWidth );
	
	ofSetColor(255,255, 255, 100 );
	
	ofPushMatrix();
	ofScale( gridScale, gridScale, gridScale );
	
	gridShader.begin();
	grid.draw(GL_LINES, 0, numGridVertices );
	
	glLineWidth( majorGridLineWidth );
	gridMajor.draw(GL_LINES, 0, numGridMajorVertices );
	
	
	glLineWidth(gridLineWidth * majorGridLineWidth * 2);
	ofSetColor(255, 55, 30, 100);
	ofLine(-1000, 0, 0, 1000, 0, 0);
	
	ofSetColor( 55, 55, 255, 100);
	ofLine( 0, 0, -1000, 0, 0, 1000);
	
	gridShader.end();
	
	
	ofPopMatrix();
	
	
	//draw camera lines
	persptiveCameraNode.setPosition( sin(ofGetElapsedTimef() ) * 200, modelTransform.getGlobalPosition().y, cos(ofGetElapsedTimef() ) * 200 );
	persptiveCameraNode.setScale(30);
	persptiveCameraNode.lookAt(modelTransform.getGlobalPosition());
	
	ofSetColor(200, 200, 200, 100);
	glLineWidth( 2 );
	ofPushMatrix();
	ofMultMatrix(persptiveCameraNode.getGlobalTransformMatrix());
	
	cameraLines.draw(GL_LINES, 0, cameraLinesNumVertices );
	
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

void CloudsVisualSystem3DModel::loadShaders()
{
	normalShader.load( getVisualSystemDataPath() + "shaders/normalShader" );
	gridShader.load( getVisualSystemDataPath() + "shaders/gridShader" );
	facingRatioShader.load( getVisualSystemDataPath() + "shaders/facingRatio" );
	
	shaderNames.clear();
	addToShaderMap( "normalShader", &normalShader );
	addToShaderMap( "gridShader", &gridShader );
	addToShaderMap( "facingRatio", &facingRatioShader );
}
			  
void CloudsVisualSystem3DModel::addToShaderMap( string name, ofShader* shader )
{
	if(shaderMap.find( name ) == shaderMap.end() )
	{
		shaderMap[ name ] = shader;
		shaderNames.push_back( name );//just used for setting up the gui
	}

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


void CloudsVisualSystem3DModel::loadModel( string fileName, bool bSmoothMesh )
{
	ofxObjLoader::load( getVisualSystemDataPath() + fileName, modelMesh, true );
	calcBoundingBox();
	
	float mScl = maxDim / max( maxBound.x - minBound.x, max(maxBound.y-minBound.y, maxBound.z - minBound.z ));
	modelScl.set( mScl, mScl, mScl );
	
	if(bSmoothMesh)
	{
		smoothMesh( modelMesh, modelMesh );
	}
	
}

string CloudsVisualSystem3DModel::vec3ToString( ofVec3f v, int precision ){
	return ofToString( v.x, precision) + "_" + ofToString( v.y, precision) + "_" + ofToString( v.z, precision);
}

ofVec3f CloudsVisualSystem3DModel::normalFrom3Points(ofVec3f p0, ofVec3f p1, ofVec3f p2)
{
	ofVec3f norm = (p2 - p1).cross( p0 - p1);
	return norm.normalized();
}

void CloudsVisualSystem3DModel::smoothMesh( ofMesh& facetedMesh, ofMesh& targetMesh, int precision)
{
	cout << "smoothing mesh" << endl;
	
	//get our vertex, uv and face info
	vector<ofVec3f>& v = facetedMesh.getVertices();
	vector<ofVec2f>& uv = facetedMesh.getTexCoords();
	vector<ofIndexType>& indices = facetedMesh.getIndices();
	bool hasTC = facetedMesh.getNumTexCoords();
	
	//use these to store our new mesh info
	map<string, unsigned int> mergeMap;
	vector<ofVec3f> smoothVertices;
	vector<ofVec3f> smoothNormals;
	vector<ofVec2f> smoothTexCoords;
	vector<ofIndexType> smoothIndices;
	
	//merge our vertices by pointing near by vertices to the same index
	for (int i=0; i<v.size(); i++)
	{
		mergeMap[ vec3ToString( v[i], precision ) ] = i;
	}
	
	//fill our smoothed vertex array with merged vertices & tex coords
	smoothVertices.resize( mergeMap.size() );
	if(hasTC)	smoothTexCoords.resize( mergeMap.size() );
	int smoothVertexCount = 0;
	for (map<string, unsigned int>::iterator it = mergeMap.begin(); it != mergeMap.end(); it++)
	{
		smoothVertices[smoothVertexCount] = v[it->second];
		if(hasTC)	smoothTexCoords[smoothVertexCount] = uv[it->second];
		it->second = smoothVertexCount;//store our new vertex index
		smoothVertexCount++;
	}
	
	
	//reconstruct our faces by reassigning their indices to the merged vertices
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
		n = normalFrom3Points( smoothVertices[smoothIndices[i]], smoothVertices[smoothIndices[i+1]], smoothVertices[smoothIndices[i+2]] );
		smoothNormals[smoothIndices[i]] += n;
		smoothNormals[smoothIndices[i+1]] += n;
		smoothNormals[smoothIndices[i+2]] += n;
	}
	
	for (int i=0; i<smoothNormals.size(); i++)
	{
		smoothNormals[i].normalize();
	}
	
	//setup our smoothed mesh. this should still work if our targetMesh is our facetedMesh
	targetMesh.clear();
	targetMesh.addVertices( smoothVertices );
	targetMesh.addNormals( smoothNormals );
	if(hasTC)	targetMesh.addTexCoords( smoothTexCoords );
	targetMesh.addIndices( smoothIndices );
}


void CloudsVisualSystem3DModel::facetMesh( ofMesh& smoothedMesh, ofMesh& targetMesh )
{
	//get our vertex, uv and face info
	vector<ofVec3f>& v = smoothedMesh.getVertices();
	vector<ofVec2f>& uv = smoothedMesh.getTexCoords();
	vector<ofIndexType>& indices = smoothedMesh.getIndices();
	bool hasTC = smoothedMesh.getNumTexCoords();
	
	//use these to store our new mesh info
	vector<ofVec3f> facetedVertices( indices.size() );
	vector<ofVec3f> facetedNormals( indices.size() );
	vector<ofVec2f> facetedTexCoords;
	if(hasTC){
		facetedTexCoords.resize( indices.size() );
	}
	vector<ofIndexType> facetedIndices( indices.size() );
	
	//store vertex and uv data
	for (int i=0; i < indices.size(); i++) {
		facetedIndices[i] = i;
		facetedVertices[i] = v[indices[i]];
		if(hasTC)	facetedTexCoords[i] = uv[indices[i]];
	}
	
	//calculate our face normals
	ofVec3f n;
	for (int i=0; i < facetedIndices.size(); i+=3) {
		n = normalFrom3Points( facetedVertices[i], facetedVertices[i+1], facetedVertices[i+2]);
		facetedNormals[i] = n;
		facetedNormals[i+1] = n;
		facetedNormals[i+2] = n;
	}
	
	//setup our faceted mesh. this should still work if our targetMesh is our smoothMesh
	targetMesh.clear();
	targetMesh.addVertices( facetedVertices );
	targetMesh.addNormals( facetedNormals );
	if(hasTC)	targetMesh.addTexCoords( facetedTexCoords );
	targetMesh.addIndices( facetedIndices );
}

void CloudsVisualSystem3DModel::resizeTheArrowMesh( float radius, float height, float pointBaseHight ){
	//the top pointy part of the mesh vertices are 1-12 but not 6.	1,...,5,7,...,12
	
	vector<ofVec3f> v = arrowMesh.getVertices();

	for (int i=0; i<v.size(); i++) {
		if(v[i].y > .1 && v[i].y < .9){
			v[i].y = pointBaseHight;
		}
		
	}
	
	arrowMesh.clearVertices();
	arrowMesh.addVertices( v );
	
	arrowScale.set( radius/2, height, radius/2 );
}


void CloudsVisualSystem3DModel::drawPerspective(){
	//draws a perspective view with our default camera
}

void CloudsVisualSystem3DModel::loadCameraLineModel( ofVbo& vbo, string loc ){
	ofBuffer buffer = ofBufferFromFile( loc );
	vbo.clear();
	vector<ofVec3f> points;
    if(buffer.size())
	{
		while (buffer.isLastLine() == false)
		{
			string line = buffer.getNextLine();
			vector<string> vals = ofSplitString( line, ",");
			
			for (int i=0; i<vals.size(); i+=3)
			{
				points.push_back( ofVec3f( ofToFloat(vals[i]), ofToFloat(vals[i+1]), ofToFloat(vals[i+2]) ) );
			}
		}
		
    }
	
	vbo.setVertexData( &points[0], points.size(), GL_STATIC_DRAW );
	cameraLinesNumVertices = points.size();
	points.clear();
	buffer.clear();
}

void CloudsVisualSystem3DModel::setupBoundingBoxVbo()
{
	
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

void CloudsVisualSystem3DModel::setupGridVbos()
{
	
	float gridDim = 100;
	float halfGridDim = gridDim / 2;
	vector<ofVec3f> gridVertices(gridDim * 4);
	for (int i=0; i<gridDim; i++)
	{
		gridVertices[i*4+0].set(i - halfGridDim, 0,-halfGridDim);
		gridVertices[i*4+1].set(i - halfGridDim, 0, halfGridDim);
		gridVertices[i*4+2].set(-halfGridDim, 0, i - halfGridDim);
		gridVertices[i*4+3].set( halfGridDim, 0, i - halfGridDim);
	}
	grid.setVertexData( &gridVertices[0], gridVertices.size(), GL_STATIC_DRAW );
	numGridVertices = gridVertices.size();
	gridVertices.clear();
	
	for (int i=0; i<gridDim; i += 5)
	{
		gridVertices.push_back( ofVec3f(i - halfGridDim, 0,-halfGridDim) );
		gridVertices.push_back( ofVec3f(i - halfGridDim, 0, halfGridDim) );
		gridVertices.push_back( ofVec3f(-halfGridDim, 0, i - halfGridDim) );
		gridVertices.push_back( ofVec3f( halfGridDim, 0, i - halfGridDim) );
	}
	gridMajor.setVertexData( &gridVertices[0], gridVertices.size(), GL_STATIC_DRAW );
	numGridMajorVertices = gridVertices.size();
	gridVertices.clear();
}







