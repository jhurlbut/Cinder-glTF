#pragma once


#include "cinder/app/app.h"
#include "GLTFNode.h"


namespace cinder {
	namespace gltf {
		typedef enum { TIME, SCALE, ROTATION, TRANSLATION, LINEAR } AnimTypes;
		class AnimSampler;
		typedef std::shared_ptr<class AnimSampler>		AnimSamplerRef;
		class AnimSampler
		{
		public:
			static AnimSamplerRef	create() { return AnimSamplerRef(new AnimSampler()); }
			AnimSampler(){};
			AnimTypes input;
			AnimTypes output;
			AnimTypes interpolation;
			float interpolate(float time, float in){
				return 2;
			};
			vec3 interpolate(float time, vec3 in){
				return vec3(0, 0, 0);
			};
			mat4 interpolate(float time, mat4 in){
				return mat4(1);
			};

		};

		class AnimChannel;
		typedef std::shared_ptr<class AnimChannel>		AnimChannelRef;
		class AnimChannel
		{
		public:
			static AnimChannelRef	create() { return AnimChannelRef(new AnimChannel()); }
			AnimChannel(){
				interpVal = 0.f;

			};
			NodeRef		target;
			AnimTypes targetPath; //rotation, translation
			AnimSamplerRef sampler;
			float interpVal;

			void updateInterp(float interval){
				interpVal = interval;
			}
			void update(AnimTypes type, float val){

			};
			void update(AnimTypes type, vec3 oldval, vec3 newval){

				if (type == targetPath){
					switch (targetPath)
					{

					case AnimTypes::TRANSLATION:
					{
						vec3 pos = glm::mix(oldval, newval, interpVal);
						target->trans = pos;
					}

						break;
					case AnimTypes::SCALE:
					{
						vec3 scaleval = glm::mix(oldval, newval, interpVal);

						target->scale = scaleval;
						//ci::app::console() << target->scale << std::endl;
						break;
					}
					}
				}
			};
			void update(AnimTypes type, quat oldval, quat val){

				if (type == targetPath){
					switch (targetPath)
					{
					case AnimTypes::ROTATION:
						quat interquat = glm::slerp(oldval, val, interpVal);
						target->rotmat = glm::mat4_cast(interquat);
						break;
					}

				}
			};
		};
		class Animation;
		typedef std::shared_ptr<class Animation>		AnimationRef;
		class Animation
		{
		public:
			static AnimationRef	create() { return AnimationRef(new Animation()); }
			Animation(){
				curKey = curTimeKeyVal = time = 0;
				looping = true;
			};

			std::map<std::string, AnimChannelRef> mChannels;
			std::map<std::string, AnimSamplerRef> mSamplers;
			std::vector<std::pair<AnimTypes, AccessorRef>> mParams; //TIME,scale
			int count;
			float curTimeKeyVal;
			float time;
			vec3 curKeyTransVal;
			vec3 curKeyScaleVal;
			quat curKeyRotVal;
			int curKey;
			bool looping;
			AccessorRef	timeAccessor;
			void update(float delta){
				for (auto parm : mParams){
					if (parm.first == AnimTypes::TIME){
						float lastTimeKeyVal = parm.second->animBuffer1[glm::max(0, curKey - 1)];
						curTimeKeyVal = parm.second->animBuffer1[curKey];
						float keyValDiff = (curTimeKeyVal - lastTimeKeyVal);
						float percDiff = 0.f;
						if (keyValDiff>0.f)
							percDiff = (time - lastTimeKeyVal) / keyValDiff;
						percDiff = glm::min(1.f, percDiff);
						for (auto chan : mChannels){
							chan.second->updateInterp(percDiff);
						}
					}
					if (parm.first == AnimTypes::TRANSLATION){
						vec3 lastKeyVal = parm.second->animBuffer3[glm::max(0, curKey - 1)];
						curKeyTransVal = parm.second->animBuffer3[curKey];
						for (auto chan : mChannels){
							chan.second->update(parm.first, lastKeyVal, curKeyTransVal);
						}
					}
					if (parm.first == AnimTypes::SCALE){
						vec3 lastKeyVal = parm.second->animBuffer3[glm::max(0, curKey - 1)];
						curKeyScaleVal = parm.second->animBuffer3[curKey];
						for (auto chan : mChannels){
							chan.second->update(parm.first, lastKeyVal, curKeyScaleVal);
						}
					}
					if (parm.first == AnimTypes::ROTATION){
						quat lastKeyVal = parm.second->animBuffer4[glm::max(0, curKey - 1)];
						curKeyRotVal = parm.second->animBuffer4[curKey];
						for (auto chan : mChannels){
							chan.second->update(parm.first, lastKeyVal, curKeyRotVal);
						}
					}

				}

				time += delta;
				if (curTimeKeyVal < time){
					if (curKey < count - 1){
						curKey++;
					}
					else {
						if (looping){
							curKey = 0;
							time = 0;
						}
					}
				}
			}
		};
	}
}