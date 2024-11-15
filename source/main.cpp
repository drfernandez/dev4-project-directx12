// Simple basecode showing how to create a window and attatch a d3d12surface
#define GATEWARE_ENABLE_CORE // All libraries need this
#define GATEWARE_ENABLE_SYSTEM // Graphics libs require system level libraries
#define GATEWARE_ENABLE_GRAPHICS // Enables all Graphics Libraries
#define GATEWARE_ENABLE_MATH // Enables all Math Libraries
#define GATEWARE_ENABLE_INPUT
// Ignore some GRAPHICS libraries we aren't going to use
#define GATEWARE_DISABLE_GDIRECTX11SURFACE // we have another template for this
#define GATEWARE_DISABLE_GOPENGLSURFACE // we have another template for this
#define GATEWARE_DISABLE_GVULKANSURFACE // we have another template for this
#define GATEWARE_DISABLE_GRASTERSURFACE // we have another template for this
// With what we want & what we don't defined we can include the API
#include "Gateware.h"
#include "renderer.h"
#include "chronotimer.h"
#include <DirectXColors.h>

// open some namespaces to compact the code a bit
using namespace GW;
using namespace CORE;
using namespace SYSTEM;
using namespace GRAPHICS;
// lets pop a window and use D3D12 to clear to a jade colored screen
int main()
{
	ChronoTimer timer;
	GWindow win;
	GEventResponder msgs;
	GDirectX12Surface d3d12;
	if (+win.Create(0, 0, 1280, 800, GWindowStyle::WINDOWEDBORDERED))
	{
		win.SetWindowName("Dan Fernandez - Project - DirectX 12");

		if (+d3d12.Create(win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT))
		{

			Renderer renderer(win, d3d12); // init
			while (+win.ProcessWindowEvents())
			{
				timer.Signal();
				if (+d3d12.StartFrame())
				{
					ID3D12GraphicsCommandList* cmd = nullptr;
					D3D12_CPU_DESCRIPTOR_HANDLE rtv = {};
					D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
					if (+d3d12.GetCommandList((void**)&cmd) && 
						+d3d12.GetCurrentRenderTargetView((void**)&rtv) &&
						+d3d12.GetDepthStencilView((void**)&dsv))
					{
						cmd->ClearRenderTargetView(rtv,	DirectX::Colors::DarkBlue, 0, nullptr);
						cmd->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1, 0, 0, nullptr);

						renderer.Update(timer.Delta());
						renderer.Render(); // draw

						d3d12.EndFrame(true);
						cmd->Release();
					}
				}
			}// clean-up when renderer falls off stack
		}
	}
	
	return 0; // that's all folks
}