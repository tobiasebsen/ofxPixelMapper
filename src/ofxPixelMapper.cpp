//
//  ofxPixelMapper.cpp
//
//  Created by Tobias Ebsen on 19/06/17.
//
//
#include "ofxPixelMapper.h"


void ofxPixelMapper::setup(PixelMode mode, int numUniverses, int numChannels) {

    this->mode = mode;
    this->numUniverses = numUniverses;
    this->numChannelsPerUniverse = numChannels;

    numChannelsPerPixel = mode == RGB ? 3 : 4;
    numPixelsPerUniverse = (int)ceilf((float)numChannelsPerUniverse / numChannelsPerPixel);
    numPixelsTotal = 0;
    
    fbo.allocate(numPixelsPerUniverse, numUniverses, mode == RGB ? GL_RGB : GL_RGBA);
    fbo.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    fbo.begin();
    ofClear(0, 0);
    fbo.end();
}

int ofxPixelMapper::getNumUniverses() {
    return numUniverses;
}

int ofxPixelMapper::getNumPixels() {
    return numPixelsTotal;
}

void ofxPixelMapper::addQuad(int pixel, int universe, float x1, float y1, float x2, float y2) {

    mesh.addTexCoord(ofVec2f(x1, y1));
    mesh.addTexCoord(ofVec2f(x2, y1));
    mesh.addTexCoord(ofVec2f(x2, y2));
    mesh.addTexCoord(ofVec2f(x1, y2));
    
    mesh.addVertex(ofVec2f(pixel, universe));
    mesh.addVertex(ofVec2f(pixel+1, universe));
    mesh.addVertex(ofVec2f(pixel+1, universe+1));
    mesh.addVertex(ofVec2f(pixel, universe+1));
    
    numPixelsTotal++;
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

void ofxPixelMapper::updateMapping() {
    vbo.setMesh(mesh, GL_STATIC_DRAW);
}

void ofxPixelMapper::update(ofTexture & tex) {
    fbo.begin();
    ofClear(0, 0, 0);
    ofDisableAlphaBlending();
    
    tex.bind();

    GLint magFilter = GL_LINEAR;
    glGetTexParameteriv(tex.texData.textureTarget, GL_TEXTURE_MAG_FILTER, &magFilter);
    GLint minFilter = GL_LINEAR;
    glGetTexParameteriv(tex.texData.textureTarget, GL_TEXTURE_MIN_FILTER, &minFilter);
    //tex.setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    glTexParameteri(tex.texData.textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(tex.texData.textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    ofSetColor(255);
    vbo.enableTexCoords();
    vbo.draw(GL_QUADS, 0, mesh.getNumVertices());
    
    tex.unbind();
    fbo.end();
    
    tex.setTextureMinMagFilter(minFilter, magFilter);
}

void ofxPixelMapper::draw(float x, float y) {
    fbo.draw(x, y);
}

void ofxPixelMapper::draw(float x, float y, float w, float h) {
    fbo.draw(x, y, w, h);
}

void ofxPixelMapper::read(int universe, unsigned char * data, int byteLength) {
    int npixels = (byteLength / numChannelsPerPixel) * numChannelsPerPixel;
    fbo.bind();
    glReadPixels(0, universe, MIN(numPixelsPerUniverse, npixels), 1, ((mode == RGB) ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
    fbo.unbind();
}

