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

#include "CinderGLTF.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Context.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "ShaderPreprocessor.h"
#include "cinder/gl/GlslProg.h"
using boost::lexical_cast;



using namespace std;
using namespace ci;
using namespace ci::app;
using namespace gl;

using boost::make_tuple;

const float DRAW_SCALE = 1;
namespace cinder {
	namespace gltf {



		CinderGLTF::CinderGLTF(DataSourceRef dataSource, int numInstances)

		{
			mNumInstances = numInstances;
			std::shared_ptr<CinderGLTF> p(this);
			pRef = p.get()->shared_from_this();

			mModelName = dataSource->getFilePath().parent_path().filename().replace_extension("").string();

			mVAO = gl::Vao::create();

			if (numInstances > 0){
				// create an array of initial per-instance positions laid out in a 2D grid

				for (size_t potX = 0; potX < numInstances; ++potX) {
					float instanceX = .015 * potX / numInstances;
					float instanceY = -0.00f;

					instancePositions.push_back(vec3(instanceX, instanceY, -.01f));
					instanceScales.push_back(vec3(.0));
				}

				// create the VBO which will contain per-instance (rather than per-vertex) data
				mInstancePosVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(vec3), instancePositions.data(), GL_DYNAMIC_DRAW);
				mInstanceScaleVbo = gl::Vbo::create(GL_ARRAY_BUFFER, instanceScales.size() * sizeof(vec3), instanceScales.data(), GL_DYNAMIC_DRAW);

				// we need a geom::BufferLayout to describe this data as mapping to the CUSTOM_0 semantic, and the 1 (rather than 0) as the last param indicates per-instance (rather than per-vertex)
				geom::BufferLayout instanceDataLayout;
				instanceDataLayout.append(geom::Attrib::CUSTOM_0, 3, 0, 0, 1 /* per instance */);


			}
			parse(dataSource);
		}
		void CinderGLTF::parse(ci::DataSourceRef pJSON){


			try{
				JsonTree doc(pJSON);

				console() << "loaded gltf json: " << pJSON->getFilePath() << ". num children " << doc.getNumChildren() << endl;
				if (doc.hasChild("buffers")){
					JsonTree buffers(doc.getChild("buffers"));
					handleBuffer(buffers);
				}
				if (doc.hasChild("shaders")){
					JsonTree shaders(doc.getChild("shaders"));
					handleShader(shaders);
				}
				if (doc.hasChild("programs")){
					JsonTree programs(doc.getChild("programs"));
					handleProgram(programs);
				}

				if (doc.hasChild("techniques")){
					JsonTree technique(doc.getChild("techniques"));
					handleTechnique(technique);
				}

				if (doc.hasChild("images")){
					JsonTree images(doc.getChild("images"));
					handleImage(images);
				}
				if (doc.hasChild("samplers")){
					JsonTree samplers(doc.getChild("samplers"));
					handleSampler(samplers);
				}
				if (doc.hasChild("textures")){
					JsonTree textures(doc.getChild("textures"));
					handleTexture(textures);
				}
				if (doc.hasChild("materials")){
					JsonTree material(doc.getChild("materials"));
					handleMaterial(material);
				}
				if (doc.hasChild("bufferViews")){
					JsonTree bufferViews(doc.getChild("bufferViews"));
					handleBufferView(bufferViews);
				}
				if (doc.hasChild("accessors")){
					JsonTree accessors(doc.getChild("accessors"));
					handleAccessor(accessors);
				}
				if (doc.hasChild("meshes")){
					JsonTree meshes(doc.getChild("meshes"));
					handleMesh(meshes);
				}
				if (doc.hasChild("nodes")){
					JsonTree nodes(doc.getChild("nodes"));
					handleNode(nodes);
				}
				if (doc.hasChild("scenes")){
					JsonTree scenes(doc.getChild("scenes"));
					handleScene(scenes);
				}
				if (doc.hasChild("animations")){
					JsonTree anim(doc.getChild("animations"));
					handleAnimation(anim);
				}
			}
			catch (cinder::JsonTree::Exception err){
				console() << "unable to load the json, what what: " << err.what() << endl;
				exit(-1);
			}


		}
		void CinderGLTF::update(float time){
			// update our instance positions; map our instance data VBO, write new positions, unmap
			if (mNumInstances > 0){
				vec3 *positions = (vec3*)mInstancePosVbo->mapWriteOnly(true);
				for (size_t potX = 0; potX < mNumInstances; ++potX) {
					vec3 newPos(instancePositions[potX]);
					*positions++ = newPos;
				}
				mInstancePosVbo->unmap();

				vec3 *scales = (vec3*)mInstanceScaleVbo->mapWriteOnly(true);
				for (size_t potX = 0; potX < mNumInstances; ++potX) {
					vec3 newScale(instanceScales[potX]);
					*scales++ = newScale;
				}
				mInstanceScaleVbo->unmap();
			}
			for (auto anim : mAnimations){
				anim.second->update(time);
			}
		}
		void CinderGLTF::draw(){
			//gl::drawCube(vec3(0, 0, 0), vec3(40.4, 40.4, 40.4));
			auto ctx = gl::Context::getCurrent();

			gl::ScopedVao vao(mVAO);
			gl::enableDepthRead();
			gl::enableDepthWrite();
			//gl::ScopedVao source(mVAO);
			//

			typedef map<string, SceneRef> MapT;
			for (MapT::value_type& pair : mScenes)
			{
				pair.second->draw(mNumInstances);
			}
		}
		void CinderGLTF::handleSkin(const JsonTree& tree)
		{

			
		}

		void CinderGLTF::handleImage(const JsonTree& tree){
			for (const JsonTree& img : tree.getChildren())
			{
				auto pth = "models/" + mModelName + "/" + img["uri"].getValue<string>();
				try{
					mImages[img.getKey()] = loadImage(loadAsset(pth));
				}
				catch (cinder::ImageIoException err){
					ci::app::console() << "image load error : " << err.what() << endl;
					exit(-1);
				}
			}
		}
		void CinderGLTF::handleAccessor(const JsonTree& tree){
			for (const JsonTree& acc : tree.getChildren())
			{

				AccessorRef a = Accessor::create();
				std::string bufName = acc["bufferView"].getValue<string>();
				a->vextexBuffer = mBufferViews[bufName]->vbo;

				a->count = acc["count"].getValue<size_t>();
				a->type = acc["componentType"].getValue<GLenum>();
				a->pointer = reinterpret_cast<const GLvoid*>(acc["byteOffset"].getValue<int>());
				string t = acc["type"].getValue<string>();
				if (t == "SCALAR"){
					a->size = 1;
				}
				if (t == "VEC2"){
					a->size = 2;
				}
				if (t == "VEC3"){
					a->size = 3;
				}
				if (t == "VEC4"){
					a->size = 4;
				}
				if (t == "MAT2"){
					a->size = 2 * 2;
				}
				if (t == "MAT3"){
					a->size = 3 * 3;
				}
				if (t == "MAT4"){
					a->size = 4 * 4;
				}
				if (acc.hasChild("max")){
					auto maxvals = acc.getChild("max");

					a->max = vec3(maxvals[0].getValue<float>(), maxvals[1].getValue<float>(), a->size == 3 ? maxvals[2].getValue<float>() : 0);
					
				}
				if (acc.hasChild("min")){
					auto maxvals = acc.getChild("min");
					a->min = vec3(maxvals[0].getValue<float>(), maxvals[1].getValue<float>(), a->size == 3 ? maxvals[2].getValue<float>() : 0);
					a->mBox = ci::AxisAlignedBox3f(a->min, a->max);
				}
				if (acc.hasChild("byteStride")){
					a->stride = acc["byteStride"].getValue<GLsizei>();
				}
				else {
					//must be animation accessor
					a->stride = 0;
					//animat buffer. read to disk
					auto datasrc = mBufferViews[bufName]->data;
					int byteOffset = acc["byteOffset"].getValue<int>();
					auto databuffer = datasrc->getBuffer();

					size_t numElements = (databuffer.getDataSize()) / sizeof(float);
					const float* pBuffer = reinterpret_cast<const float*>(databuffer.getData());

					std::vector<float> floatData;
					for (size_t i = 0; i<a->count*a->size; ++i) {
						floatData.push_back(pBuffer[i + (byteOffset / sizeof(float))]); // or uint32_t for the other case
						//ci::app::console() << pBuffer[i + byteOffset] << endl;
					}

					int size = a->size;
					switch (size){
					case(1) :
						a->animBuffer1 = floatData;
						if (a->animBuffer1.size() != a->count)
							console() << "data not count size " << endl;
						break;
					case(2) :
					{
						std::vector<vec2> data2;
						for (int i = 0; i < floatData.size(); i += a->size) {
							data2.push_back(vec2(floatData[i], floatData[i + 1]));
						}
						if (data2.size() != a->count)
							console() << "data2 not count size " << endl;
					}
							break;
					case(3) :
					{
						std::vector<vec3> data3;
						for (int i = 0; i < floatData.size(); i += a->size) {
							data3.push_back(vec3(floatData[i], floatData[i + 1], floatData[i + 2]));
						}
						if (data3.size() != a->count)
							console() << "data3 not count size " << endl;
						a->animBuffer3 = data3;
					}
							break;
					case(4) :
					{
						std::vector<vec4> data4;
						for (int i = 0; i < floatData.size(); i += a->size) {
							data4.push_back(vec4(floatData[i], floatData[i + 1], floatData[i + 2], floatData[i + 3]));
						}
						std::vector<quat> quats;
						for (vec4 val : data4){
							//have to reverse the rotation around the axis for some reason
							quat q = fromAngleAxis(val.a, vec3(val.x, val.y, val.z));
							quats.push_back(q);
						}
						if (quats.size() != a->count)
							console() << "quats not count size " << endl;
						a->animBuffer4 = quats;
					}
							break;
					}
				}

				mAccessors[acc.getKey()] = a;
			}
		}
		void CinderGLTF::handleBuffer(const JsonTree& tree)
		{
			for (const JsonTree& buf : tree.getChildren())
			{
				size_t byteLength = buf["byteLength"].getValue<size_t>();
				string path = buf["uri"].getValue();
				auto pth = "models/" + buf.getKey() + "/" + path;
				console() << "key " << buf.getKey() << endl;
				DataSourceRef dataSrc = ci::app::loadAsset(pth);
				if (dataSrc->getBuffer().getDataSize() != byteLength)
				{
					console() << buf.getKey() << ": byteLength mismatch." << endl;
				}
				mBuffers[buf.getKey()] = dataSrc;
			}
		}
		void CinderGLTF::handleShader(const JsonTree& tree)
		{
			for (const JsonTree& shader : tree.getChildren())
			{
				ShaderRef shd = Shader::create();
				shd->type = shader["type"].getValue<GLint>();
				auto pth = "models/" + mModelName + "/" + shader["uri"].getValue<string>();
				shd->data = loadAsset(pth);
				mShaders[shader.getKey()] = shd;
			}
		}

		void CinderGLTF::handleProgram(const JsonTree& tree)
		{

			for (const JsonTree& prog : tree.getChildren())
			{
				cinder::gl::ShaderPreprocessor pp;
				//mAttributes[i] = gl::Vao::create();
				ProgramRef pgm = Program::create();
				pgm->vertShader = prog["vertexShader"].getValue<string>();
				pgm->fragShader = prog["fragmentShader"].getValue<string>();
				try {
					pgm->shader = gl::GlslProg::create(
						gl::GlslProg::Format()
						.vertex(pp.parse(mShaders[pgm->vertShader]->data->getFilePath()).c_str())
						.fragment(pp.parse(mShaders[pgm->fragShader]->data->getFilePath()).c_str())
						.attrib(geom::Attrib::CUSTOM_0, "vInstancePosition")
						.attrib(geom::Attrib::CUSTOM_1, "vInstanceScale")
						.fragDataLocation(0, "oColor")
						.fragDataLocation(1, "BloomColor")
						);

				}
				catch (cinder::gl::ShaderPreprocessorExc err){
					console() << "ShaderPreprocessorExc error what " << err.what() << endl;
					exit(-1);
				}
				catch (GlslProgExc err){

					console() << "vert source: " << pp.parse(mShaders[pgm->vertShader]->data->getFilePath()).c_str() << endl;
					console() << "frag source: " << pp.parse(mShaders[pgm->fragShader]->data->getFilePath()).c_str() << endl;
					console() << "GLSL error what " << err.what() << endl;

					exit(-1);
				}
				for (const JsonTree& att : prog["attributes"].getChildren())
				{
				}
				mPrograms[prog.getKey()] = pgm;
			}
		}
		void CinderGLTF::handleTexture(const JsonTree& tree)
		{
			for (const JsonTree& tex : tree.getChildren())
			{
				const Surface& image = mImages[tex["source"].getValue()];

				gl::Texture::Format& format = mSamplers[tex["sampler"].getValue()];
				format.setTarget(tex["target"].getValue<GLenum>());
				format.setInternalFormat(tex["internalFormat"].getValue<GLenum>());

				gl::TextureRef texture;

				texture = gl::Texture::create(image, format);
				texture->setTopDown(true);
				mTextures[tex.getKey()] = texture;

			}
		}
		void CinderGLTF::handleTechnique(const JsonTree& tree)
		{

			for (const JsonTree& techjson : tree.getChildren())
			{
				TechniqueRef tech = Technique::create();
				tech->name = techjson.getKey();

				string passName = techjson["pass"].getValue();
				const JsonTree& defaultPassTree = techjson["passes"][passName];
				auto instanceProgram = defaultPassTree.getChild("instanceProgram");
				InstanceProgramRef iPgm = InstanceProgram::create();

				ProgramRef pgm = mPrograms[instanceProgram.getChild("program").getValue<string>()];
				for (auto atts : instanceProgram.getChild("uniforms")){
					iPgm->uniforms[atts.getValue<string>()] = atts.getKey();
				}
				for (auto atts : instanceProgram.getChild("attributes")){
					iPgm->attributes[atts.getKey()] = atts.getValue<string>();
				}
				iPgm->program = pgm;
				tech->instanceProgram = iPgm;

				for (auto parms : techjson["parameters"].getChildren()){
					ParamRef param = Param::create();
					param->type = parms["type"].getValue<GLint>();
					//if (param->type == GL_FLOAT){
					//	param->valF = parms["value"].getValue<float>();
					//}
					if (param->type == GL_FLOAT_MAT4){
						if (parms.hasChild("source")){
							//NodeRef node = mNodes[parms["source"].getValue<string>()];
							param->matSource = parms["source"].getValue<string>();
						}
					}
					if (param->type == GL_FLOAT_VEC3){
						if (parms.hasChild("value")){
							auto val = parms["value"];
							param->val = vec4(val[0].getValue<float>(), val[1].getValue<float>(), val[2].getValue<float>(), 0);
						}
						else {
							param->val = vec4(0, 0, 0, 0);

						}
					}
					if (param->type == GL_FLOAT_VEC4){
						if (parms.hasChild("value")){
							auto val = parms["value"];
							param->val = vec4(val[0].getValue<float>(), val[1].getValue<float>(), val[2].getValue<float>(), val[3].getValue<float>());
						}
						else {
							param->val = vec4(0, 0, 0, 0);

						}
					}
					if (parms.hasChild("semantic")){
						param->semantic = parms["semantic"].getValue<string>();
					}
					tech->params[parms.getKey()] = param;
				}


				//tech->blendEnable = defaultPassTree["states"]["blendEnable"].getValue<bool>();
				//tech->blendEnable = defaultPassTree["states"]["cullFaceEnable"].getValue<bool>();
				//tech->blendEnable = defaultPassTree["states"]["depthMask"].getValue<bool>();
				//tech->blendEnable = defaultPassTree["states"]["depthTestEnable"].getValue<bool>();

				mTechniques[tech->name] = tech;
			}
		}
		void CinderGLTF::handleMesh(const JsonTree& tree)
		{
			for (const JsonTree& meshjson : tree.getChildren())
			{
				MeshRef mesh = Mesh::create();
				mesh->name = meshjson.getKey();

				for (const JsonTree& primitive : meshjson["primitives"].getChildren())
				{
					PrimitiveRef prim = Primitive::create();
					if (mNumInstances > 0){
						prim->mInstancePosVbo = mInstancePosVbo;
						prim->mInstanceScaleVbo = mInstanceScaleVbo;
					}
					prim->pIndexBuffer = mAccessors[primitive["indices"].getValue()];
					prim->pMaterial = mMaterials[primitive["material"].getValue()];

					for (const JsonTree& attrib : primitive["attributes"].getChildren())
					{
						std::string attname = attrib.getKey();

						if (attname == "POSITION"){
							prim->pVertexBuffers.push_back(make_pair(geom::Attrib::POSITION, mAccessors[attrib.getValue()]));
						}
						if (attname == "TEXCOORD_0"){
							prim->pVertexBuffers.push_back(make_pair(geom::Attrib::TEX_COORD_0, mAccessors[attrib.getValue()]));
						}if (attname == "NORMAL"){
							prim->pVertexBuffers.push_back(make_pair(geom::Attrib::NORMAL, mAccessors[attrib.getValue()]));
						}

					}

					//prim.pSkin = &mSkins[primitive["skin"].getValue()];

					mesh->primitives.push_back(prim);
				}

				mMeshes[mesh->name] = mesh;
			}
		}
		void CinderGLTF::handleMaterial(const JsonTree& tree)
		{
			for (const JsonTree& matjson : tree.getChildren())
			{
				MaterialRef material = Material::create();
				material->name = matjson.getKey();

				string techniqueName = matjson["instanceTechnique"]["technique"].getValue();
				TechniqueRef technique = mTechniques[techniqueName];
				TechniqueRef techniqueInstance = Technique::create();
				material->pTechnique = technique;
				material->pTechInstance = techniqueInstance;
				for (const JsonTree& value : matjson["instanceTechnique"]["values"].getChildren())
				{
					string paramName = value.getKey();
					auto params = technique->params;
					ParamRef param = Param::create();

					vec4 val;
					string key = value.getKey();

					InstanceProgramRef ipg = material->pTechnique->instanceProgram;
					auto prm = params[paramName];
					if (prm->type == GL_FLOAT){
						prm->valF = value.getValue<float>();
						param->valF = value.getValue<float>();
					}
					if (prm->type == GL_FLOAT_VEC4){
						vec4 val;
						int i = 0;
						for (auto num : value.getChildren()){
							float floatval = num.getValue<float>();
							val[i] = floatval;
							i++;
						}
						prm->val = val;
						param->val = val;
					}
					if (prm->type == GL_FLOAT_VEC2){
						vec4 val;
						int i = 0;
						for (auto num : value.getChildren()){
							float floatval = num.getValue<float>();
							val[i] = floatval;
							i++;
						}
						prm->val = val;
						param->val = val;
					}
					if (prm->type == GL_FLOAT_VEC3){
						//ipg.program.shader->uniform(ipg.uniforms[paramName], vec3(val.x,val.y,val.z));
						vec4 val;
						int i = 0;
						for (auto num : value.getChildren()){
							float floatval = num.getValue<float>();
							val[i] = floatval;
							i++;
						}
						val.w = 0;
						prm->val = val;
						param->val = val;
					}

					if (prm->type == GL_SAMPLER_2D){
						material->mUseTextures = true;
						string texName = value.getValue();
						gl::TextureRef tex = mTextures[texName];
						InstanceProgramRef ipg = material->pTechnique->instanceProgram;
						if (tex)
							material->textures.push_back(make_pair(ipg->uniforms[paramName], tex));


					}
					if (prm->type == GL_FLOAT_MAT4){
						NodeRef node = mNodes[value["source"].getValue<string>()];
						ci::mat4 matrix = node->matrix;
						prm->matval = matrix;
						param->matval = matrix;
						//InstanceProgram ipg = material->pTechnique->instanceProgram;
						//material->uniforms.push_back(make_pair(ipg.uniforms[paramName], node.matrix));
					}
					//	continue;
					//}
					techniqueInstance->params[paramName] = param;
					// TODO: support more types

				}
				material->shader = material->pTechnique->instanceProgram->program->shader;
				mMaterials[material->name] = material;
			}
		}
		void CinderGLTF::handleNode(const JsonTree& tree)
		{
			for (const JsonTree& nodejson : tree.getChildren()){
				NodeRef node = Node::create();
				node->name = nodejson.getKey();

				for (const JsonTree& child : nodejson["children"].getChildren())
				{
					handleNode(child);
				}

				size_t i = 0;
				float matvals[16];
				if (nodejson.hasChild("matrix")){
					for (const JsonTree& number : nodejson["matrix"].getChildren())
					{
						matvals[i] = number.getValue<float>();
						i++;
					}
					mat4 mat = glm::make_mat4(matvals);
					DecomposedType type;
					Decompose(mat, type);
					node->trans = vec3(type.translateX, type.translateY, type.translateZ);
					node->scale = vec3(type.scaleX, type.scaleY, type.scaleZ);
					node->rotmat = glm::mat4_cast(quat(type.quaternionW, type.quaternionX, type.quaternionY, type.quaternionZ));
					node->matrix = mat;
				}
				else {
					//create a matrix from the srt

					vec3 scale;
					vec3 trans;

					size_t i = 0;
					float vals[4];
					for (const JsonTree& number : nodejson["rotation"].getChildren())
					{
						vals[i] = number.getValue<float>();
						i++;
					}
					quat q = fromAngleAxis(vals[3], vec3(vals[0], vals[1], vals[2]));

					i = 0;
					for (const JsonTree& number : nodejson["scale"].getChildren())
					{
						scale[i] = number.getValue<float>();
						i++;
					}
					i = 0;
					for (const JsonTree& number : nodejson["translation"].getChildren())
					{
						trans[i] = number.getValue<float>();
						i++;
					}

					mat4 nodemat(1);
					mat4 scalemat = glm::scale(scale);
					mat4 transmat = glm::translate(trans);
					mat4 rotmat = glm::mat4_cast(q);

					//nodemat = glm::scale(nodemat, scale);
					//nodemat *= glm::mat4_cast(rot);
					nodemat = glm::translate(nodemat, trans);
					//ci::app::console() << " : trans " << glm::vec3(nodemat[3]) << std::endl;
					node->matrix = transmat * rotmat * glm::scale(scale);
					node->rotmat = rotmat;
					node->scale = scale;
					node->trans = trans;
				}
				if (nodejson.hasChild("meshes"))
				{
					for (const JsonTree& mesh : nodejson["meshes"].getChildren())
					{
						console() << "push mesh " << mesh.getValue() << endl;
						if (mesh.getValue() == "ID5539"){
							console() << "debug " << endl;
						}
						node->pMeshes.push_back(mMeshes[mesh.getValue()]);
						PrimitiveRef prim = node->pMeshes[0]->primitives[0];
						bool hasPosBuf = false;
						for (auto buf : prim->pVertexBuffers){
							if (buf.first == geom::Attrib::POSITION){
								AccessorRef acc = buf.second;
								ci::AxisAlignedBox3f bnds = acc->mBox;
								node->bounds = bnds;
								hasPosBuf = true;
							}
						}
						if (hasPosBuf == false){
							node->bounds = ci::AxisAlignedBox3f(vec3(0), vec3(0));
						}
					}
				}
				else {
					node->bounds = ci::AxisAlignedBox3f(vec3(0),vec3(0));

				}
				if (!mNodes[node->name]){
					mNodes[node->name] = node;
				}
				{
					console() << "dupe node found: " << node->name << endl;
				}
			}
			for (const JsonTree& nodejson : tree.getChildren()){
				NodeRef node = mNodes[nodejson.getKey()];

				for (const JsonTree& child : nodejson["children"].getChildren())
				{
					node->pChildren.push_back(mNodes[child.getValue()]);
				}

			}
		}
		void CinderGLTF::handleScene(const JsonTree& tree)
		{
			for (const JsonTree& scenejson : tree.getChildren()){
				SceneRef scene = Scene::create();

				mNodeNames.push_back(kAllNodeName);
				for (const JsonTree& node : scenejson["nodes"].getChildren())
				{
					scene->pNodes.push_back(mNodes[node.getValue()]);
					mNodeNames.push_back(node.getValue());
				}

				mScenes[tree.getKey()] = scene;
			}
		}
		void CinderGLTF::handleSampler(const JsonTree& tree)
		{
			for (const JsonTree& samp : tree.getChildren())
			{
				gl::Texture::Format format;
				format.setMagFilter(samp["magFilter"].getValue<GLenum>());
				format.setMinFilter(samp["minFilter"].getValue<GLenum>());
				format.setWrapS(samp["wrapS"].getValue<GLenum>());
				format.setWrapT(samp["wrapT"].getValue<GLenum>());
				format.enableMipmapping();
				mSamplers[samp.getKey()] = format;
			}
		}
		void CinderGLTF::handleBufferView(const JsonTree& tree)
		{
			for (const JsonTree& buf : tree.getChildren())
			{
				gl::VboRef vbo;
				BufferViewRef bView = BufferView::create();

				size_t byteLength = buf["byteLength"].getValue<size_t>();
				size_t byteOffset = buf["byteOffset"].getValue<size_t>();

				string bufferName = buf["buffer"].getValue();
				DataSourceRef& dataSrc = mBuffers[bufferName];
				bView->data = dataSrc;
				const Buffer& buffer = dataSrc->getBuffer();

				//VBO buffer
				if (buf.hasChild("target")){
					vbo = gl::Vbo::create(buf["target"].getValue<GLenum>());

					//vbo->bind();
					vbo->bufferData(byteLength, reinterpret_cast<const uint8_t*>(buffer.getData()) + byteOffset, GL_STATIC_DRAW);
					vbo->unbind();
					bView->vbo = vbo;
				}
				mBufferViews[buf.getKey()] = bView;
			}
		}
		void CinderGLTF::handleAnimation(const JsonTree& tree)
		{
			for (const JsonTree& animjson : tree.getChildren()){
				AnimationRef anim = Animation::create();
				if (animjson.hasChild("samplers")){
					for (const JsonTree& sampler : animjson["samplers"].getChildren())
					{
						AnimSamplerRef samp = AnimSampler::create();
						AnimTypes input;
						AnimTypes output;
						AnimTypes interp;

						string interpolation = sampler["interpolation"].getValue<string>();
						if (interpolation == "LINEAR")
							interp = AnimTypes::LINEAR;

						string in = sampler["input"].getValue<string>();
						if (in == "TIME")
							input = AnimTypes::TIME;

						string out = sampler["output"].getValue<string>();
						if (out == "scale")
							output = AnimTypes::SCALE;
						if (out == "rotation")
							output = AnimTypes::ROTATION;
						if (out == "translation")
							output = AnimTypes::TRANSLATION;

						samp->input = input;
						samp->output = output;
						samp->interpolation = interp;
						anim->mSamplers[sampler.getKey()] = samp;
					}
				}
				int count = animjson["count"].getValue<int>();
				if (animjson.hasChild("parameters")){
					for (const JsonTree& param : animjson["parameters"].getChildren())
					{
						AnimTypes path;
						AccessorRef	acc;
						acc = mAccessors[param.getValue<string>()];
						string p = param.getKey();
						count = animjson["count"].getValue<int>();
						if (p == "TIME")
							path = AnimTypes::TIME;
						if (p == "scale")
							path = AnimTypes::SCALE;
						if (p == "rotation")
							path = AnimTypes::ROTATION;
						if (p == "translation"){
							path = AnimTypes::TRANSLATION;

							if (acc->animBuffer3.size() < count){
								console() << "count larger than size of buffer! " << animjson.getKey() << endl;
								count = acc->animBuffer3.size();
							}
						}
						anim->mParams.push_back(make_pair(path, acc));
					}
				}
				if (animjson.hasChild("channels")){
					for (const JsonTree& chan : animjson["channels"].getChildren())
					{
						AnimChannelRef ch = AnimChannel::create();
						string samplerName = chan.getChild("sampler").getValue<string>();
						string targetNode = chan.getChild("target").getChild("id").getValue<string>();
						ch->sampler = anim->mSamplers[samplerName];
						ch->target = mNodes[targetNode];
						string tPath = chan.getChild("target").getChild("path").getValue<string>();
						AnimTypes path;
						if (tPath == "scale")
							path = AnimTypes::SCALE;
						if (tPath == "rotation")
							path = AnimTypes::ROTATION;
						if (tPath == "translation")
							path = AnimTypes::TRANSLATION;

						ch->targetPath = path;
						anim->mChannels[chan.getKey()] = ch;
					}
				}


				anim->count = count;
				mAnimations[animjson.getKey()] = anim;
			}
		}
	}
}