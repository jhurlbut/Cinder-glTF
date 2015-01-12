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
			ci::AxisAlignedBox3f mBox;
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