// STD
use std::rc::Rc;
use std::sync::Arc;
use std::collections::HashSet;

// EXTERN CRATES
use vulkano::device::{Device, DeviceExtensions, Queue, QueuesIter};
use vulkano::image::SwapchainImage;
use vulkano::instance;
use vulkano::instance::{ApplicationInfo, Instance, PhysicalDevice, QueueFamily};
use vulkano::swapchain::{PresentMode, Surface, SurfaceTransform, Swapchain};
use vulkano::pipeline::shader::ShaderModule;
use vulkano::buffer::{CpuAccessibleBuffer, BufferView, DeviceLocalBuffer, BufferUsage};
use vulkano::command_buffer::{AutoCommandBuffer, AutoCommandBufferBuilder};
use vulkano::framebuffer::RenderPass;
use vulkano::pipeline::GraphicsPipeline;
use vulkano_win::{required_extensions, VkSurfaceBuild};

use specs::{System, ReadStorage, Read, storage::{VecStorage, DenseVecStorage, HashMapStorage, NullStorage}};

use winit::{EventsLoop, Window, WindowBuilder};
use winit::dpi::{LogicalSize, PhysicalSize};

use smallvec::SmallVec;


// OTHER MODULES
use math::*;
use volume::*;


/// Info that (nearly) all shaders need, pulled out into a global uniform. Will be UBO 0
#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct PerFrameInfo {
    timeStep: f32,
    persp: Perspective,
    camPos: Mat4x4,
    // TODO: Anything else we can pull out as global?
}

/// Common voxel vert. Should be handle to cover all cases with some clever shader magic.
#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct VoxelVert {
    pub pos: Vec3,
    pub data: Voxel,
}


pub struct Renderer {
    windowDims: [u32; 2],

    instance: Arc<Instance>,
    physicalIndex: usize,
    logical: Arc<Device>,

    window: Arc<Surface<Window>>,
    swapchain: Arc<Swapchain<Window>>,
    swapImages: Vec<Arc<SwapchainImage<Window>>>,
}

impl Renderer {
    pub fn new() -> (EventsLoop, Renderer) {
        let instance = {
            let appInfo = app_info_from_cargo_toml!();

            Instance::new(Some(&appInfo), &required_extensions(), None)
                .expect("Failed to create a vulkan instance!")
        };

        let mut windowDims = [1600, 900];
        let evLoop = EventsLoop::new();
        let window: Arc<Surface<Window>> = WindowBuilder::new()
            .with_dimensions(LogicalSize::from_physical(
                PhysicalSize::new(windowDims[0] as f64, windowDims[1] as f64),
                1.0,
            ))
            .with_title("GLENgine-rs")
            .build_vk_surface(&evLoop, instance.clone())
            .expect("Failed to create a window/vulkan surface!");


        let mut physicalIndex: usize = 0;
        let (logical, mut queues): (Arc<Device>, QueuesIter) = {
            let physical: PhysicalDevice = PhysicalDevice::enumerate(&instance)
                .filter(|dev: &PhysicalDevice| {
                    if dev.ty() != instance::PhysicalDeviceType::DiscreteGpu {
                        return false;
                    }

                    return true;
                })
                .next()
                .expect("Failed to find a suitable device!");
            physicalIndex = physical.index();

            println!(
                "Using device: {} (type: {:?})",
                physical.name(),
                physical.ty()
            );


            // TODO: Dynamically doing 1+ queue families
            let qFam = physical.queue_families().find(|q| q.supports_graphics() && q
                .supports_compute() && window.is_supported(*q).unwrap())
                .expect("Failed to find a suitable queue family!");

            let qFams = vec![(qFam, 1.0 as f32)];

            let deviceExt = DeviceExtensions {
                khr_swapchain: true,
                ..DeviceExtensions::none()
            };


            Device::new(
                physical,
                physical.supported_features(),
                &deviceExt,
                qFams,
            ).expect("Failed to create a logical device!")
        };

        let queue = queues.next().expect("Failed to get a queue!");

        let (mut swapchain, mut swapImages) = {
            let physical = PhysicalDevice::from_index(&instance, physicalIndex).unwrap();

            let caps = window
                .capabilities(physical)
                .expect("Failed to check window capabilities!");

            let alpha = caps.supported_composite_alpha.iter().next().unwrap();
            windowDims = caps.current_extent.unwrap_or(windowDims);

            let format = caps.supported_formats[0].0;

            Swapchain::new(
                logical.clone(),
                window.clone(),
                caps.min_image_count,
                format,
                windowDims,
                1,
                caps.supported_usage_flags,
                &queue,
                SurfaceTransform::Identity,
                alpha,
                PresentMode::Mailbox,
                true,
                None,
            ).expect("Failed to create a swapchain!")
        };


        return (evLoop, Renderer {
            windowDims,
            instance,
            physicalIndex,
            logical,
            window,
            swapchain,
            swapImages,
        },
        );
    }

    // TODO: Make this more acceptable for multithreading
    pub fn render(&self) {}

    pub fn get_window(&self) -> &Window {
        self.window.window()
    }
}


impl<'a> System<'a> for Renderer {
    type SystemData = (ReadStorage<'a, components::Camera>,
                       ReadStorage<'a, components::Material>,
                       ReadStorage<'a, Volume>);

    fn run(&mut self, data: Self::SystemData) {
        let (cameras, materials, volumes) = data;

        let x: Mat4x4;
    }
}


pub type ViewportID = usize;
pub type PipelineID = usize;
pub type TextureID = usize;

pub mod components {
    use super::*;

    #[derive(Debug, Clone, Component)]
    #[storage(HashMapStorage)]
    pub struct Camera {
        persp: Perspective,
        renderTo: ViewportID,
    }

    #[derive(Debug, Clone, Component)]
    #[storage(DenseVecStorage)]
    pub struct Material {
        pipeline: PipelineID,
        textures: SmallVec<[TextureID; 2]>,

    }
}