/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../common/blenders.h"
#include "../common/color.h"
#include "../common/paths.h"

#include <agge/clipper.h>
#include <agge/filling_rules.h>
#include <agge/path.h>
#include <agge/rasterizer.h>
#include <agge/renderer_parallel.h>
#include <agge/stroke.h>
#include <agge/stroke_features.h>

#include <jni.h>
#include <android/bitmap.h>
#include <memory>

using namespace agge;
using namespace common;
using namespace std;

namespace
{
	pixel32 make_pixel(rgba8 color)
	{
		pixel32 p = { color.r, color.g, color.b, 0 };
		return p;
	}

	template <typename BlenderT>
	class blenderx : public BlenderT
	{
	public:
		typedef typename BlenderT::cover_type cover_type;

	public:
		blenderx(rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}
	};

	class bitmap_proxy
	{
	public:
		typedef pixel32 pixel;

	public:
		bitmap_proxy(JNIEnv *env, jobject bitmap)
			: _env(env), _bitmap(bitmap), _memory(NULL)
		{
			AndroidBitmapInfo info;

			if (AndroidBitmap_getInfo(env, bitmap, &info) < 0 || info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
				throw 0;
			_width = info.width;
			_height = info.height;
			_stride = info.stride / sizeof(pixel);
			if (AndroidBitmap_lockPixels(env, bitmap, &_memory) < 0 || !_memory)
				throw 0;
		}

		~bitmap_proxy()
		{
			AndroidBitmap_unlockPixels(_env, _bitmap);
		}

		pixel *row_ptr(unsigned y)
		{	return reinterpret_cast<pixel *>(_memory) + y * _stride;	}

		unsigned width() const
		{	return _width;	}

		unsigned height() const
		{	return _height;	}

	private:
		bitmap_proxy(const bitmap_proxy &other);
		const bitmap_proxy &operator =(const bitmap_proxy &other);

	private:
		JNIEnv * const _env;
		const jobject _bitmap;
		unsigned _width, _height, _stride;
		void *_memory;
	};

	typedef blenderx<blender_solid_color> blender;

	struct AGG : noncopyable
	{
	public:
		AGG(count_t parallelism)
			: renderer(parallelism)
		{	}

	public:
		agge::rasterizer< clipper<int> > rasterizer;
		renderer_parallel renderer;
		AggPath spiral;
		agge::stroke stroke;
	};
}



extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_constructAGG(JNIEnv *env, jobject obj)
try
{
	AGG *ptr = new AGG(1);
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	env->SetLongField(obj, fieldidAGG, reinterpret_cast<jlong>(ptr));
}
catch (...)
{
}

extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_destroyAGG(JNIEnv *env, jobject obj)
{
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	delete reinterpret_cast<AGG*>(env->GetLongField(obj, fieldidAGG));
	env->SetLongField(obj, fieldidAGG, 0);
}

extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_render(JNIEnv *env, jobject obj, jobject bitmap)
try
{
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	AGG* agg = reinterpret_cast<AGG*>(env->GetLongField(obj, fieldidAGG));

	bitmap_proxy bm(env, bitmap);

	agg_path_adaptor p(agg->spiral);

	agg->stroke.width(3);
	agg->stroke.set_cap(agge::caps::butt());
	agg->stroke.set_join(agge::joins::bevel());

	agge::path_generator_adapter<agg_path_adaptor, agge::stroke> stroke_path(p, agg->stroke);

	agg->rasterizer.reset();
	add_path(agg->rasterizer, stroke_path);
	agg->rasterizer.sort();
	agg->renderer(bm, 0, agg->rasterizer, blender(rgba8(0, 154, 255, 255)), agge::winding<>());
}
catch (...)
{
}

extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_updateSize(JNIEnv *env, jobject obj, jint width, jint height)
try
{
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	AGG* agg = reinterpret_cast<AGG*>(env->GetLongField(obj, fieldidAGG));

	agg->spiral.clear();
	spiral(agg->spiral, width / 2, height / 2, 5, (std::min)(width, height) / 2 - 10, 1, 0);
}
catch (...)
{
}
