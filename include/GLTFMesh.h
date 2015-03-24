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
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/gl/Batch.h"

#include "cinder/gl/VboMesh.h"
#include "cinder/GeomIo.h"
#include "cinder/gl/Context.h"

#include "GLTFShading.h"
#include "GLTFBuffers.h"

namespace cinder {
	namespace gltf {
		class Primitive;
		typedef std::shared_ptr<class Primitive>		PrimitiveRef;
		class Primitive
		{
		public:
			static PrimitiveRef	create() { return PrimitiveRef(new Primitive()); }
			Primitive(){};
			typedef std::pair<ci::geom::Attrib, AccessorRef> NameAttribPair;

			AccessorRef      pIndexBuffer;
			MaterialRef   pMaterial;
			std::vector<NameAttribPair> pVertexBuffers;
			gl::VboRef	mInstancePosVbo;
			gl::VboRef	mInstanceScaleVbo;
			void draw(int numInstances, std::vector<ci::mat4> rotMats, std::vector<ci::mat4> pTrans, std::vector<mat4> scales)
			{
				//double t0 = ci::app::getElapsedSeconds();
				pMaterial->preDraw(rotMats,pTrans,scales);
				//double t1 = ci::app::getElapsedSeconds();
				//ci::app::console() << (t1 - t0) * 1000 << ":"<<pMaterial->name <<": predraw time " << std::endl;
				auto ctx = ci::gl::Context::getCurrent();
				auto curGlslProg = ctx->getGlslProg();
				if (numInstances > 0){
					int loc = curGlslProg->getAttribSemanticLocation(geom::Attrib::CUSTOM_0);
					if (loc != -1){
						mInstancePosVbo->bind();
						gl::enableVertexAttribArray(loc);
						//3, 0, 0, 1 /* per instance */ );
						//uint8_t dims, size_t stride, size_t offset, uint32_t instanceDivisor = 0 ) {
						gl::vertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
						ctx->vertexAttribDivisor(loc, 1);
					}
					loc = curGlslProg->getAttribSemanticLocation(geom::Attrib::CUSTOM_1);
					if (loc != -1){
						mInstanceScaleVbo->bind();
						gl::enableVertexAttribArray(loc);
						//3, 0, 0, 1 /* per instance */ );
						//uint8_t dims, size_t stride, size_t offset, uint32_t instanceDivisor = 0 ) {
						gl::vertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
						ctx->vertexAttribDivisor(loc, 1);
					}
					
				}
				for (NameAttribPair& pair : pVertexBuffers)
				{
					//t0 = ci::app::getElapsedSeconds();
					int loc = curGlslProg->getAttribSemanticLocation(pair.first);
					pair.second->preDraw(loc);
					int test = 3;
					//t1 = ci::app::getElapsedSeconds();
					//ci::app::console() << (t1 - t0) * 1000 << ":" << pair.first << ":vert buffers predraw time " << std::endl;

				}
				
					
				//t0 = ci::app::getElapsedSeconds();
				pIndexBuffer->vextexBuffer->bind();
				ctx->setDefaultShaderVars();
				if (numInstances == 0){
					gl::drawElements(GL_TRIANGLES, pIndexBuffer->count, pIndexBuffer->type, pIndexBuffer->pointer);
				}
				else {
					glDrawElementsInstanced(GL_TRIANGLES, pIndexBuffer->count, pIndexBuffer->type, pIndexBuffer->pointer, numInstances);
				}
				pIndexBuffer->vextexBuffer->unbind();
				//t1 = ci::app::getElapsedSeconds();
				
				//ci::app::console() << (t1 - t0) * 1000 << ": draw pIndexBuffer time " << std::endl;
				//t0 = ci::app::getElapsedSeconds();
				for (NameAttribPair& pair : pVertexBuffers)
				{
					int loc = curGlslProg->getAttribSemanticLocation(pair.first);
					pair.second->postDraw(loc);
				}
				pMaterial->postDraw();
				if (numInstances > 0){
					mInstancePosVbo->unbind();
					mInstanceScaleVbo->unbind();
				}
				//t1 = ci::app::getElapsedSeconds();

				//ci::app::console() << (t1 - t0) * 1000 << " : material/vbuffer postdraw time " << std::endl;

			}
		};

		class Mesh;
		typedef std::shared_ptr<class Mesh>		MeshRef;
		class Mesh
		{
		public:
			static MeshRef	create() { return MeshRef(new Mesh()); }
			Mesh(){};

			std::string      name;

			ci::gl::BatchRef		mBatch;


			std::vector<PrimitiveRef> primitives;

			void draw(int numInstances, std::vector<mat4> rotMat, std::vector<mat4> trans, std::vector<mat4> scales)
			{
				for (PrimitiveRef prim : primitives)
				{
					//double t0 = ci::app::getElapsedSeconds();
					prim->draw(numInstances, rotMat, trans, scales);
					//double t1 = ci::app::getElapsedSeconds();
					//ci::app::console() << (t1 - t0) * 1000 << " : " << name << " : prim draw time " << std::endl;

					
				}
			}
		};

	}
}