/*
 *
 * Copyright (c) 2014, James Hurlbut
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of James Hurlbut nor the names of its
 * contributors may be used to endorse or promote products
 * derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "CinderGLTF.h"
#include "cinder/Camera.h"
#include "cinder/CameraUI.h"

#include "cinder/params/Params.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/GeomIo.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Batch.h"
#include "cinder/Rand.h"
#include "cinder/Log.h"
#include "cinder/Color.h"

#include "glm/gtx/euler_angles.hpp"
#include "cinder/params/Params.h"
#include "ShaderPreprocessor.h"

#include "cinder/params/Params.h"

using namespace ci;
using namespace ci::app;
using namespace std;

struct LightData {
    bool						toggleViewpoint;
    float						distanceRadius;
    float						fov;
    CameraPersp					camera;
    vec3						viewpoint;
    vec3						target;
};

class BasicGLTFApp : public App {
public:
    void setup() override;
    void mouseDown(MouseEvent event) override;
    void mouseDrag(MouseEvent event) override;
    void keyDown(KeyEvent event) override;
    void prepareSettings(Settings *settings);
    void update() override;
    void draw() override;
    LightData					mLight;
    gltf::CinderGLTFRef         gltfmesh;
    CameraPersp					mCamera;
    CameraUi					mMayaCamera;
    float						mTime;
    float						mDelta;
    float						mLastTime;
    params::InterfaceGlRef		mParams;
    Color						mLightColor;
};
void BasicGLTFApp::prepareSettings(Settings *settings)
{
    settings->setWindowSize(1024, 768);
    settings->setTitle("Cinder GLTF");
}
void BasicGLTFApp::setup()
{
    
    auto model = loadAsset("models/animation/animation.gltf");
    gltfmesh = gltf::CinderGLTF::create(model);
    // setup camera and lights
    mCamera.lookAt({10,10,10}, {0,0,0});
    mCamera.setNearClip(.1f);
    mCamera.setFarClip(100.0f);
    mCamera.setAspectRatio(getWindowAspectRatio());
    
    mMayaCamera = CameraUi(&mCamera);
    
    mTime = mDelta = mLastTime = 0.f;
    
    mLightColor = Color::white();
    
    //associate the transform matrices with the materials
    for (auto mat : gltfmesh->mMaterials){
        mat.second->nodeMap = gltfmesh->mNodes;
    }
    
    mParams = params::InterfaceGl::create("CinderGLTF", ivec2(300, 400));
    mParams->addSeparator();
    mParams->addText("Lights");
    
    mParams->addParam("Direct Light Color", &mLightColor).updateFn([&]{
        auto tech = gltfmesh->mTechniques["technique1"];
        gltf::ParamRef param = (*tech->params)["light0Color"];
        param->val = {mLightColor.r, mLightColor.g, mLightColor.b, 0.f};
    });
    
}

void BasicGLTFApp::keyDown(KeyEvent event)
{
    
}

void BasicGLTFApp::mouseDown(MouseEvent event)
{
    mMayaCamera.mouseDown(event.getPos());
}

void BasicGLTFApp::mouseDrag(MouseEvent event)
{
    mMayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
    mCamera = mMayaCamera.getCamera();
}

void BasicGLTFApp::update()
{
    mDelta = getElapsedSeconds() - mLastTime;
    mLastTime = getElapsedSeconds();
    gltfmesh->update(mDelta);
    
}

void BasicGLTFApp::draw()
{
    gl::clear(Color::gray(.6));
    gl::pushMatrices();
    gl::setMatrices(mCamera);
    
    gl::enableDepthRead();
    gl::enableDepthWrite();
    try{
        gltfmesh->draw();
    }
    catch (...){
        ci::app::console() << "exception" << endl;
    }
    gl::popMatrices();
    
    mParams->draw();
}

CINDER_APP(BasicGLTFApp, RendererGl);
