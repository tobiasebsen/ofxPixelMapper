//
//  ofxPixelMapper.cpp
//
//  Created by Tobias Ebsen on 19/06/17.
//
//
#include "ofxPixelMapper.h"

ofxPixelMapper::ofxPixelMapper() {
    useNormalized = false;
    useCalib = false;
}

void ofxPixelMapper::setup(PixelMode mode, int numUniverses, int numChannels) {

    this->mode = mode;
    this->numUniverses = numUniverses;
    this->numChannelsPerUniverse = numChannels;

    numChannelsPerPixel = mode == RGB ? 3 : 4;
    numPixelsPerUniverse = (int)ceilf((float)numChannelsPerUniverse / numChannelsPerPixel);
    numPixelsTotal = 0;
    
    fbo.allocate(numPixelsPerUniverse, numUniverses, mode == RGB ? GL_RGB : GL_RGBA, 0);
    fbo.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    fbo.begin();
    ofClear(0, 0);
    fbo.end();
    
    brightness = 1.f;
}

int ofxPixelMapper::getNumUniverses() {
    return numUniverses;
}

int ofxPixelMapper::getNumPixels() {
    return numPixelsTotal;
}

int ofxPixelMapper::getNumChannelsPerUniverse() {
    return numChannelsPerUniverse;
}

int ofxPixelMapper::getNumPixelsPerUniverse() {
    return numPixelsPerUniverse;
}

void ofxPixelMapper::setBrightness(float bright) {
    this->brightness = bright;
}

void ofxPixelMapper::setUseCalibration(bool useCalib) {
    this->useCalib = useCalib;
}

void ofxPixelMapper::addQuad(int pixel, int universe, ofVec2f v1, ofVec2f v2, ofVec2f v3, ofVec2f v4, ofFloatColor calib) {
    
    mesh.addTexCoord(v1);
    mesh.addTexCoord(v2);
    mesh.addTexCoord(v3);
    mesh.addTexCoord(v4);
    
    mesh.addVertex(ofVec2f(pixel, universe));
    mesh.addVertex(ofVec2f(pixel+1, universe));
    mesh.addVertex(ofVec2f(pixel+1, universe+1));
    mesh.addVertex(ofVec2f(pixel, universe+1));
    
    mesh.addColor(calib);
    mesh.addColor(calib);
    mesh.addColor(calib);
    mesh.addColor(calib);
    
    numPixelsTotal++;
}


void ofxPixelMapper::addQuad(int pixel, int universe, float x1, float y1, float x2, float y2) {
    
    addQuad(pixel, universe, ofVec2f(x1, y1), ofVec2f(x2, y1), ofVec2f(x2, y2), ofVec2f(x1, y2));
}

void ofxPixelMapper::addQuad(int pixel, int universe, ofRectangle rect) {
    addQuad(pixel, universe, rect.x, rect.y, rect.x+rect.width, rect.y+rect.height);
}

void ofxPixelMapper::addGrid(int pixel, int universe, float x1, float y1, float x2, float y2, int hcount, int vcount) {

    float dx = (x2 - x1) / hcount;
    float dy = (y2 - y1) / vcount;
    
    for (int y=0; y<vcount; y++) {
        for (int x=0; x<hcount; x++) {
            float xx = x1 + x * dx;
            float yy = y1 + y * dy;
            addQuad(pixel, universe, xx, yy, xx+dx, yy+dy);
            pixel++;
        }
    }
}

void ofxPixelMapper::addGrid(int pixel, int universe, ofRectangle rect, int hcount, int vcount) {
    addGrid(pixel, universe, rect.x, rect.y, rect.x+rect.width, rect.y+rect.height, hcount, vcount);
}

void ofxPixelMapper::updateMapping() {
    vbo.setMesh(mesh, GL_STATIC_DRAW);
}

void ofxPixelMapper::normalizeMapCoords(int width, int height) {
    vector<ofVec2f> & mapping = mesh.getTexCoords();
    for (ofVec2f & m : mapping) {
        m.x /= (float)width;
        m.y /= (float)height;
    }
    useNormalized = true;
}

void ofxPixelMapper::update(ofTexture & tex) {
    fbo.begin();
    ofClear(0, 0, 0);
    
    glEnable(tex.texData.textureTarget);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(tex.texData.textureTarget, tex.texData.textureID);

    GLint magFilter = GL_LINEAR;
    glGetTexParameteriv(tex.texData.textureTarget, GL_TEXTURE_MAG_FILTER, &magFilter);
    GLint minFilter = GL_LINEAR;
    glGetTexParameteriv(tex.texData.textureTarget, GL_TEXTURE_MIN_FILTER, &minFilter);
    //tex.setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    GLenum err = glGetError();
    glTexParameteri(tex.texData.textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(tex.texData.textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    err = glGetError();
    
    vbo.enableTexCoords();

    if (useCalib)
        vbo.enableColors();
    else {
        vbo.disableColors();
        ofSetColor(brightness * 255.f);
    }
    vbo.draw(GL_QUADS, 0, mesh.getNumVertices());
    ofSetColor(255);
    
    glBindTexture(tex.texData.textureTarget, 0);
    glDisable(tex.texData.textureTarget);
            
    fbo.end();
    
    tex.setTextureMinMagFilter(minFilter, magFilter);
}

void ofxPixelMapper::draw(float x, float y, float scale) {
    fbo.draw(x, y, fbo.getWidth() * scale, fbo.getHeight() * scale);
}

void ofxPixelMapper::drawMapping() {
    vector<ofVec2f> & mapping = mesh.getTexCoords();
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, mapping.data());
    glDrawArrays(GL_QUADS, 0, mapping.size());
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ofxPixelMapper::drawMapping(ofTexture & tex) {
    vector<ofVec2f> & mapping = mesh.getTexCoords();
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, mapping.data());
    tex.bind();
    drawMapping();
    tex.unbind();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void ofxPixelMapper::read(int universe, unsigned char * data, int byteLength) {
    int npixels = (byteLength / numChannelsPerPixel) * numChannelsPerPixel;
    fbo.bind();
    glReadPixels(0, universe, MIN(numPixelsPerUniverse, npixels), 1, ((mode == RGB) ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
    fbo.unbind();
}

void ofxPixelMapper::read(ofPixels & pixels) {
    fbo.readToPixels(pixels);
}

