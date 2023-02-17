#include "framegraph_resource.h"

namespace light::fg
{
	void FrameGraphResource::Create()
	{
		concept_model->Create();
	}

	void FrameGraphResource::Destroy()
	{
		concept_model->Destroy();
	}


}