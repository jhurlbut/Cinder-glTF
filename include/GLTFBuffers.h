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
#include "cinder/gl/VboMesh.h"

namespace cinder {
	namespace gltf {


		class BufferView;
		typedef std::shared_ptr<class BufferView>		BufferViewRef;
		class BufferView
		{
		public:
			static BufferViewRef	create() { return BufferViewRef(new BufferView()); }
			BufferView(){};

			DataSourceRef data;
			gl::VboRef vbo;

		};

		class Accessor;
		typedef std::shared_ptr<class Accessor>		AccessorRef;
		class Accessor
		{
		public:
			static AccessorRef	create() { return AccessorRef(new Accessor()); }
			Accessor(){};

			gl::VboRef vextexBuffer;
			std::vector<float> animBuffer1;
			std::vector<vec3> animBuffer3;
			std::vector<quat> animBuffer4;
			geom::Attrib attrib;
			//GLuint index;
			GLint size;
			GLenum type;
			GLboolean normalized;
			GLsizei stride;
			vec3 min;
			vec3 max;
			ci::AxisAlignedBox mBox;
			const GLvoid* pointer;

			const GLvoid* indices;
			size_t count;   // The number of attributes referenced by this accessor
			// not used in glVertexAttribPointer

			void preDraw(GLuint index)
			{
				if (index == -1)
					return;

				vextexBuffer->bind();
				gl::enableVertexAttribArray(index);
				gl::vertexAttribPointer(index, size, type, GL_FALSE, stride, pointer);
			}

			void postDraw(GLuint index)
			{
				if (index == -1)
					return;

				//gl::disableVertexAttribArray(index);
				vextexBuffer->unbind();
			}

		};


	}
}