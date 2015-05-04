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

#include "cinder/app/app.h"
#include "cinder/gl/Shader.h"
#include "GLTFNode.h"
#include "cinder/Timeline.h"
#include "cinder/gl/Texture.h"
namespace cinder {
	namespace gltf {

		typedef std::pair<std::string, ci::gl::TextureRef> NameTexturePair;
		typedef std::pair<std::string, ci::vec4> NameValuePair;
		typedef std::pair<std::string, ci::mat4> NameMatPair;
		typedef std::map<std::string, GLenum> NameEnumMap;
		class Param;
		typedef std::shared_ptr<class Param>		ParamRef;
		using ParamMap = std::map < std::string, ParamRef >;
		using ParamMapRef = std::shared_ptr<ParamMap>;

		class Param
		{
		public:
			static ParamRef	create() { return ParamRef(new Param()); }
			Param(){
				val = ci::vec4(0.f, 0.f,0.f,0.f);
				valF = 0.f;
				valS = "";
				matval = ci::mat4(1);
				valueBool = false;
				
			};
			GLint type;
			int count;
			bool valueBool;
			ci::Anim<vec4> val;
			ci::Anim<mat4> matval;
			std::string matSource;
			float valF;
			std::string valS;
			std::string semantic;
		};

		class Program;
		typedef std::shared_ptr<class Program>		ProgramRef;
		class Program
		{
		public:
			static ProgramRef	create() { return ProgramRef(new Program()); }
			Program(){};

			gl::GlslProgRef shader;
			std::map<std::string, std::string> attributes;
			std::string vertShader;
			std::string fragShader;

		};
		class InstanceProgram;
		typedef std::shared_ptr<class InstanceProgram>		InstanceProgramRef;
		class InstanceProgram
		{
		public:
			static InstanceProgramRef	create() { return InstanceProgramRef(new InstanceProgram()); }
			InstanceProgram(){};

			ProgramRef program;
			std::map<std::string, std::string> attributes;
			std::map<std::string, std::string> uniforms;
		};

		class Technique;
		typedef std::shared_ptr<class Technique>		TechniqueRef;
		class Technique
		{
		public:
			static TechniqueRef	create() { return TechniqueRef(new Technique()); }
			Technique() : cullFaceEnable(false), blendEnable(true){
				params = std::make_shared<ParamMap>();
			};

			InstanceProgramRef instanceProgram;
			std::map<std::string, std::string> uniforms;
			std::string name;
			ParamMapRef params;
			bool blendEnable;
			bool cullFaceEnable;
			bool depthMask;
			bool depthTestEnable;

			void preDraw() const
			{
				

			}
			void postDraw()
			{
				//instanceProgram.program.shader->
			}
		};
		
		class Shader;
		typedef std::shared_ptr<class Shader>		ShaderRef;
		class Shader
		{
		public:
			static ShaderRef	create() { return ShaderRef(new Shader()); }
			Shader(){};

			ci::DataSourceRef	data;
			GLint				type;
		};

		class Material;
		typedef std::shared_ptr<class Material>		MaterialRef;
		class Material
		{
		public:
			static MaterialRef	create() { return MaterialRef(new Material()); }
			Material(){ 
				mUseTextures = false;
				curLevel = 0;
			};
			std::string name;
			TechniqueRef pTechnique;
			TechniqueRef pTechInstance;
			// vector of Texture ref
			std::vector<NameTexturePair> textures;
			NodeMapRef nodeMap;
			int curLevel;
			bool mUseTextures;
			gl::GlslProgRef shader;
			void preDraw(std::vector<ci::mat4> rotMats, std::vector<ci::mat4> pTrans, std::vector<ci::mat4> scales){
				
				curLevel++;
				if (mUseTextures){
					GLuint texSlot = 0;
					for (NameTexturePair& pair : textures)
					{
						pair.second->bind(texSlot);
						GLint loc = pTechnique->instanceProgram->program->shader->getUniformLocation(pair.first);
						glUniform1i(loc, texSlot);
						texSlot++;
					}
					
				}
				InstanceProgramRef ipg = pTechnique->instanceProgram;
				
				ci::gl::Context::getCurrent()->pushGlslProg(shader);
				for (auto pair : ipg->uniforms)
				{
					GLint type;
					float val;
					vec4 val4;
					NodeRef node;
					
					type = pTechnique->params->at(pair.first)->type;
					auto itr = pTechInstance->params->find(pair.first);
					if (itr != pTechInstance->params->end()){
						val = pTechInstance->params->at(pair.first)->valF;
						val4 = pTechInstance->params->at(pair.first)->val;
					}
					else {
						val = pTechnique->params->at(pair.first)->valF;
						val4 = pTechnique->params->at(pair.first)->val;
					}
					
					if (type == GL_FLOAT_MAT4){
						auto techniqueParam = pTechnique->params->at(pair.first);
						auto itr = nodeMap->find(techniqueParam->matSource);
						if (itr != nodeMap->end()){
							node = nodeMap->at(techniqueParam->matSource);
							shader->uniform(pair.second, node->matrix);
						}
						
					}
					std::string name = pair.second;
					if (type == GL_FLOAT){
						shader->uniform(pair.second, val);
					}
					if (type == GL_FLOAT_VEC4){
						shader->uniform(pair.second, val4);
					}
					if (type == GL_FLOAT_VEC3){
						shader->uniform(pair.second, vec3(val4.x, val4.y, val4.z));
					}
					if (type == GL_FLOAT_VEC2){
						shader->uniform(pair.second, vec2(val4.x, val4.y));
					}
					
				}
				//matrix xforms
				/*int count = rotMats.size();
				if (count>0){
					shader->uniform("rotMats", &rotMats[0],rotMats.size());
					shader->uniform("trans", &pTrans[0], pTrans.size());
					shader->uniform("scales", &scales[0], scales.size());
				}
				shader->uniform("numLevels", count);
				pTechnique->preDraw();*/

			}
			void postDraw()
			{
				ci::gl::Context::getCurrent()->popGlslProg();
				GLuint texSlot = 0;
				for (NameTexturePair& pair : textures)
				{
					pair.second->unbind(texSlot);
					texSlot++;
				}

			}
		};
	}
}