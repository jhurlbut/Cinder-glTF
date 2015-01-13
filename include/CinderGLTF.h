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

#pragma once

#include "cinder/TriMesh.h"
#include "cinder/Stream.h"
#include "cinder/GeomIo.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"

#include "cinder/Timeline.h"
#include "cinder/app/app.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/Json.h"
#include "cinder/gl/gl.h"
#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <map>

#include "cinder/gl/Shader.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/VboMesh.h"

#include "GLTFShading.h"
#include "GLTFBuffers.h"
#include "GLTFMesh.h"
#include "GLTFNode.h"
#include "GLTFAnimation.h"
#include "GLTFUtils.h"

namespace cinder {
	namespace gltf {
		
	static GLenum getDataType(const std::string& name)
	{
		if (name == "VEC3"){

		}
		// GL_BYTE, GL_UNSIGNED_BYTE, GL_SHORT, GL_UNSIGNED_SHORT, GL_FIXED, or GL_FLOAT
		// TODO:
		return GL_FLOAT;
	}

	
	class CinderGLTF; 
	typedef boost::shared_ptr<class CinderGLTF>		CinderGLTFRef;

	/** \brief Loads .gltf file format
	*
	*
	* \n Example usage:
	* \code

	* \endcode
	**/
	class CinderGLTF : public std::enable_shared_from_this<CinderGLTF>  {
	
	public: 
		const std::string kAllNodeName = "ALL";
		void draw();
		void update(float delta);
		CinderGLTF(DataSourceRef dataSource, int numInstances);
		std::shared_ptr<CinderGLTF> pRef;
		gl::Texture2dRef		mShadowMap;
		CinderGLTF getRef()
		{
			
		}

		static CinderGLTFRef	create(DataSourceRef dataSource, int numInstances = 0) { return CinderGLTFRef(new CinderGLTF(dataSource, numInstances)); }

		struct Camera
		{
			ci::mat4	projMatrix;
			std::vector<Node*> pChildren;
			std::string pName;
		};

		class Scene;
		typedef std::shared_ptr<class Scene>		SceneRef;
		class Scene
		{
		public:
			static SceneRef	create() { return SceneRef(new Scene()); }
			Scene(){};
		 
			std::vector<NodeRef> pNodes;

			void draw(int numInstances)
			{
				for (NodeRef pNode : pNodes)
				{
					std::vector<mat4> rotMats;
					std::vector<mat4> trans;
					std::vector<mat4> scales;
					pNode->draw(false,numInstances, rotMats,trans,scales);
				}
			}
		};


		
		std::map<std::string, DataSourceRef>              mBuffers;
		std::map<std::string, BufferViewRef>                    mBufferViews;
		std::map<std::string, std::vector<float>>			mAnimBufferViews;
		std::map<std::string, gl::Texture::Format>        mSamplers; // TODO: sampler object
		std::map<std::string, Surface>                    mImages;
		std::map<std::string, gl::TextureRef>                mTextures;
		std::map<std::string, TechniqueRef>                  mTechniques;
		std::map<std::string, MaterialRef>                   mMaterials;
		std::map<std::string, AccessorRef>                      mAccessors;
		std::map<std::string, MeshRef>                       mMeshes;
		std::map<std::string, NodeRef>                       mNodes;
		std::map<std::string, SceneRef>                      mScenes;
		std::map<std::string, ShaderRef>                      mShaders;
		std::map<std::string, ProgramRef>                      mPrograms;
		std::map<std::string, AnimationRef>                      mAnimations;
		std::vector<std::string>          mNodeNames;
		std::vector<vec3> instancePositions;
		std::vector<ci::Anim<vec3>> instanceScales;
	private:
		
		GLenum getGlEnum(const ci::JsonTree& tree, const std::string& childKeyName)
		{
			const std::string& childKeyValue = tree[childKeyName].getValue();
			return getGlEnum(childKeyValue);
		}
		GLenum getGlEnum(const std::string& name)
		{
			NameEnumMap::const_iterator it = mGlNameMap.find(name);
			if (it != mGlNameMap.end())
			{
				return it->second;
			}

			ci::app::console() << "Unsupported enum: " << name << std::endl;
			return GL_NONE;
		}
		NameEnumMap             mGlNameMap;

		void parse(ci::DataSourceRef pJSON);
		void handleBuffer(const ci::JsonTree& tree);
		void handleBufferView(const ci::JsonTree& tree);
		void handleAccessor(const ci::JsonTree& tree);
		void handleImage(const ci::JsonTree& tree);
		void handleSampler(const ci::JsonTree& tree);
		void handleShader(const ci::JsonTree& tree);
		void handleProgram(const ci::JsonTree& tree);
		void handleTechnique(const ci::JsonTree& tree);
		void handleMaterial(const ci::JsonTree& tree);
		void handleMesh(const ci::JsonTree& tree);
		void handleDefault(const ci::JsonTree& tree);
		void handleAtribute(const ci::JsonTree& tree);
		void handleNode(const ci::JsonTree& tree);
		void handleScene(const ci::JsonTree& tree);
		void handleSkin(const ci::JsonTree& tree);
		void handleIndex(const ci::JsonTree& tree);
		void handleTexture(const ci::JsonTree& tree);
		void handleAnimation(const ci::JsonTree& tree);

		std::string				mModelName;
		
		gl::VboRef	mInstancePosVbo;
		gl::VboRef	mInstanceScaleVbo;

		gl::VaoRef		mVAO;
		gl::BatchRef	mBatch;

		int		mNumInstances;
		
};
}}