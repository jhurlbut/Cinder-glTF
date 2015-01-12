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