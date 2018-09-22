// STD
use std::rc::Rc;
use std::sync::Arc;
use std::collections::HashSet;

// EXTERN CRATES
use vulkano::{
    self as vk,
    sync::GpuFuture,
    device::{Device, DeviceExtensions, Queue, QueuesIter},
    image::SwapchainImage,
    instance::{self, ApplicationInfo, Instance, PhysicalDevice, QueueFamily},
    swapchain::{PresentMode, Surface, SurfaceTransform, Swapchain},
    pipeline::{
        shader::ShaderModule,
        vertex::{Vertex, VertexDefinition},
        viewport::{Viewport, ViewportsState, Scissor},
        GraphicsPipeline, GraphicsPipelineAbstract, GraphicsPipelineBuilder,
    },
    framebuffer::{RenderPass, RenderPassAbstract, Subpass, Framebuffer, FramebufferAbstract},
    command_buffer::AutoCommandBufferBuilder as CmdBuffBuilder,
};

use vulkano_win::{required_extensions, VkSurfaceBuild};
use vulkano_shader_derive;

use specs::{
    Entities, System, ReadStorage, Read, WriteStorage,
    storage::{VecStorage, DenseVecStorage, HashMapStorage, NullStorage},
    world::{World, Bundle},
};

use winit::{EventsLoop, Window, WindowBuilder};
use winit::dpi::{LogicalSize, PhysicalSize};

use smallvec::SmallVec;


// OTHER MODULES
use scene::transform::*;
use scene::DeltaTime;
use math::*;
use volume::*;
use mesh::*;
use defaultpipeline as dp;


/// Info that (nearly) all shaders need, pulled out into a global uniform. Will be UBO 0
#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct PerFrameInfo {
    timeStep: f32,
    persp: Perspective,
    camPos: Mat4x4,
    // TODO: Anything else we can pull out as global?
}


pub struct Renderer {
    windowDims: UVec2,

    instance: Arc<Instance>,
    physicalIndex: usize,
    logical: Arc<Device>,

    window: Arc<Surface<Window>>,
    swapchain: Arc<Swapchain<Window>>,
    swapImages: Vec<Arc<SwapchainImage<Window>>>,

    renderpass: Arc<RenderPassAbstract + Send + Sync>,
    defaultPipeline: Arc<GraphicsPipelineAbstract + Send + Sync>,

    framebuffers: Option<Vec<Arc<FramebufferAbstract + Send + Sync>>>,
    prevFrame: Box<GpuFuture + Send + Sync>,

    dirtySwapchain: bool,

    dynState: vk::command_buffer::DynamicState,
    depthBuffer: Arc<vk::image::attachment::AttachmentImage<vk::format::D16Unorm>>,
}

impl Renderer {
    pub fn new() -> (EventsLoop, Renderer) {
        println!("Creating Vulkan instance...");
        let instance = {
            let appInfo = app_info_from_cargo_toml!();

            Instance::new(Some(&appInfo), &required_extensions(), None)
                .expect("Failed to create a vulkan instance!")
        };

        let mut windowDims = vec2(1600u32, 900u32);
        let evLoop = EventsLoop::new();

        println!("Creating Window...");
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
            println!("Finding a suitable physical device...");
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


            println!("Finding suitable queue family...");
            // TODO: Dynamically doing 1+ queue families
            let qFam = physical.queue_families().find(|q| q.supports_graphics() && q
                .supports_compute() && window.is_supported(*q).unwrap())
                .expect("Failed to find a suitable queue family!");

            let qFams = vec![(qFam, 1.0 as f32)];

            let deviceExt = DeviceExtensions {
                khr_swapchain: true,
                ..DeviceExtensions::none()
            };


            println!("Creating logical device...");
            Device::new(
                physical,
                physical.supported_features(),
                &deviceExt,
                qFams,
            ).expect("Failed to create a logical device!")
        };

        let queue = queues.next().expect("Failed to get a queue!");

        println!("Creating the swapchain...");
        let (swapchain, swapImages) = {
            let physical = PhysicalDevice::from_index(&instance, physicalIndex).unwrap();

            let caps = window
                .capabilities(physical)
                .expect("Failed to check window capabilities!");

            let alpha = caps.supported_composite_alpha.iter().next().unwrap();
            windowDims = caps.current_extent.unwrap_or(windowDims.into()).into();

            let format = caps.supported_formats[0].0;

            Swapchain::new(
                logical.clone(),
                window.clone(),
                caps.min_image_count,
                format,
                windowDims.into(),
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

        println!("Creating the main renderpass...");
        let renderpass = Arc::new(
            single_pass_renderpass!(logical.clone(),
                attachments: {
                    color: {
                        load: Clear,
                        store: Store,
                        format: swapchain.format(),
                        samples: 1,
                    },
                    depth: {
                        load: Clear,
                        store: DontCare,
                        format: vk::format::Format::D16Unorm,
                        samples: 1,
                    }
                },
                pass: {
                    color: [color],
                    depth_stencil: {depth}
                }
                ).unwrap()
        );

        println!("Creating the default graphics pipeline...");
        let defaultPipeline = Arc::new(GraphicsPipeline::start()
            .vertex_input(vk::pipeline::vertex::TwoBuffersDefinition::<VertPos, VertVoxel>::new())
            .vertex_shader(dp::defaultvs::Shader::load(logical.clone())
                               .expect("Failed to create vertex shader!").main_entry_point(), ())
            .triangle_list()
            .viewports_dynamic_scissors_irrelevant(1)
            .fragment_shader(dp::defaultfs::Shader::load(logical.clone())
                                 .expect("Failed to create fragment shader!").main_entry_point(), ())
            .depth_stencil_simple_depth()
            .render_pass(Subpass::from(renderpass.clone(), 0).unwrap())
            .build(logical.clone())
            .expect("Failed to create the default pipeline!")
        );

        let prevFrame = Box::new(vk::sync::now(logical.clone()));


        let dynState = vk::command_buffer::DynamicState {
            line_width: None,
            viewports: Some(vec![
                Viewport {
                    origin: [0.0, 0.0],
                    dimensions: [windowDims.x as _, windowDims.y as _],
                    depth_range: 0.0..1.0,
                }
            ]),
            scissors: None,
        };

        let depthBuffer = vk::image::AttachmentImage::transient(logical.clone(), windowDims.into(),
                                                                vk::format::D16Unorm)
            .expect("Failed to create depthBuffer!");

        return (evLoop, Renderer {
            windowDims,
            instance,
            physicalIndex,
            logical,
            window,
            swapchain,
            swapImages,
            renderpass,
            defaultPipeline,
            framebuffers: None,
            prevFrame,
            dirtySwapchain: true,
            dynState,
            depthBuffer,
        },
        );
    }

    // TODO: Make this more acceptable for multithreading
    pub fn render(&mut self) {}

    pub fn get_window(&self) -> Arc<Surface<Window>> {
        return self.window.clone();
    }
}


impl<'a> System<'a> for Renderer {
    type SystemData = (
        Read<'a, DeltaTime>,
        Entities<'a>,
        ReadStorage<'a, components::Camera>,
        ReadStorage<'a, Volume>
    );

    fn run(&mut self, data: Self::SystemData) {
        let (delta, ents, cameras, volumes) = data;


        self.prevFrame.cleanup_finished();

        if self.dirtySwapchain {
            let physDevice = PhysicalDevice::from_index(&self.instance, self.physicalIndex)
                .expect("Failed to get a previously cached physical device!");

            self.windowDims = self.window.capabilities(physDevice)
                .expect("Failed to get surface capabilities!")
                .current_extent
                .unwrap_or([1600, 900]).into();

            let (newSwapchain, newImages) = match self.swapchain
                .recreate_with_dimension(self.windowDims.into()) {
                Ok(new) => new,
                Err(vk::swapchain::SwapchainCreationError::UnsupportedDimensions) => {
                    return;
                }
                Err(err) => panic!("{:?}", err)
            };

            self.swapchain = newSwapchain;
            self.swapImages = newImages;

            self.depthBuffer = vk::image::AttachmentImage::transient(self.logical.clone(),
                                                                     self.windowDims.into(),
                                                                     vk::format::D16Unorm)
                .expect("Failed to create depthBuffer!");

            self.framebuffers = None;

            self.dynState.viewports = Some(vec![
                Viewport {
                    origin: [0.0, 0.0],
                    dimensions: [self.windowDims.x as _, self.windowDims.y as _],
                    depth_range: 0.0..1.0,
                }
            ]);

            self.dirtySwapchain = false;
        }

        if self.framebuffers.is_none() {
            let newBuffers = self.swapImages.iter()
                .map(|image| {
                    let res: Arc<FramebufferAbstract + Send + Sync> = Arc::new(Framebuffer::start(self
                        .renderpass.clone())
                        .add(image.clone()).unwrap()
                        .add(self.depthBuffer.clone()).unwrap()
                        .build().unwrap()
                    );
                    res
                }).collect();
            self.framebuffers = Some(newBuffers);
        }

        for cam in cameras.iter() {
            let persp = cam.persp;

            let uniBuffSubBuff = {
                let elapsed = (*delta).into() as f32;
                let uniData = dp::defaultvs::ty::PerFrameInfo {
                    timeStep: elapsed,
                    persp,
                    camPos: Mat4x4::identity(),
                };
            };
        }
    }
}


#[derive(Debug, Copy, Clone, NewType)]
pub struct ViewportID(usize);

#[derive(Debug, Copy, Clone, NewType)]
pub struct PipelineID(usize);

#[derive(Debug, Copy, Clone, NewType)]
pub struct TextureID(usize);

pub trait MeshData {}

pub type MesherFunc = Fn(&Chunk, &mut components::Mesh);

pub fn data_mesher(chunk: &Chunk, mesh: &mut components::Mesh){
    /*

    1256 -> +Z
    3276 -> +X
    1243 -> +Y

       0--------------1
      /|             /|
     / |            / |
    3--+-----------2  |
    |  |           |  |
    |  |           |  |
    |  |           |  |
    |  4-----------+--5
    | /            | /
    |/             |/
    7--------------6

    */
    const DIST_TO_WALL: f32 = 1.0 / ((CHUNK_SIZE * 2) as f32);
    let VERTS: [Vec3; 8] = [
        vec3(-DIST_TO_WALL, DIST_TO_WALL, DIST_TO_WALL),   // 0
        vec3(DIST_TO_WALL, DIST_TO_WALL, DIST_TO_WALL),    // 1
        vec3(DIST_TO_WALL, DIST_TO_WALL, -DIST_TO_WALL),   // 2
        vec3(-DIST_TO_WALL, DIST_TO_WALL, -DIST_TO_WALL),  // 3
        vec3(-DIST_TO_WALL, -DIST_TO_WALL, DIST_TO_WALL),  // 4
        vec3(DIST_TO_WALL, -DIST_TO_WALL, DIST_TO_WALL),   // 5
        vec3(DIST_TO_WALL, -DIST_TO_WALL, -DIST_TO_WALL),  // 6
        vec3(-DIST_TO_WALL, -DIST_TO_WALL, -DIST_TO_WALL), // 7
    ];

    let meshData: Vec<VertVoxel> = Vec::new();

    // 1 block is 1/32 of the chunk. Dist from center of block to wall of block is thus
    // (1/32)/2 = (1/64)


    for z in 0..CHUNK_SIZE {
        for y in 0..CHUNK_SIZE {
            for x in 0..CHUNK_SIZE {
                let midOfBlock = vec3(
                    ((x + 1) as f32) / CHUNK_SIZE as f32,
                    ((y + 1) as f32) / CHUNK_SIZE as f32,
                    ((z + 1) as f32) / CHUNK_SIZE as f32,
                );

                let voxel = chunk.blocks[x][y][z];

                /// Simple code-dupe reduction. Takes midOfBlock and res implicitly, as well as a
                /// list of 1+ indices (taking from the cube art above) to add
                macro_rules! vox_vertex {
                    ($index: expr) => {
                        res.push(((midOfBlock + VERTS[$index]).into(), voxel.into()));
                    };

                    ($($index: expr),*) => {
                        $(
                            vox_vertex!($index);
                        )*
                    };
                }

                // Note: Each of the following invocations of vox_vertex is doing 1 full
                // quad, with each triangle using the right hand rule for which face.
                // Reserving before each macro to try and get a *little* extra performance.

                // Always make sure we already have enough space for the worst case scenario.
                // Should give at least a mild performance bump
                res.reserve(36);

                if x == (CHUNK_SIZE - 1) || chunk.blocks[x + 1][y][z] == 0 { // +X Quad
                    vox_vertex!(5,1,2,  5,2,6);
                }
                if x == 0 || chunk.blocks[x - 1][y][z] == 0 { // -X Quad
                    vox_vertex!(7,3,4,  4,3,0);
                }


                if y == (CHUNK_SIZE - 1) || chunk.blocks[x][y + 1][z] == 0 { // +Y Quad
                    vox_vertex!(2,1,0,  3,2,0);
                }
                if y == 0 || chunk.blocks[x][y - 1][z] == 0 {
                    vox_vertex!(7,4,5,  7,5,6);
                }


                if z == (CHUNK_SIZE - 1) || chunk.blocks[x][y][z + 1] == 0 { // +Z Quad
                    vox_vertex!(5,4,0,  1,5,0);
                }
                if z == 0 || chunk.blocks[x][y][z - 1] == 0 { // -Z Quad
                    vox_vertex!(6,2,3,  7,6,3);
                }
            }
        }
    }

    res
}

pub mod components {
    use super::*;

    #[derive(Debug, Clone, Component)]
    #[storage(HashMapStorage)]
    pub struct Camera {
        persp: Mat4x4,
        renderTo: ViewportID,
    }

    impl Camera {
        pub fn new(renderTo: ViewportID, fov: f32, near: f32, far: f32) -> Camera {
            return Camera {
                persp: cg::perspective(Deg(fov), 16.0 / 9.0, near, far),
                renderTo,
            };
        }
    }

    pub use mesh::VoxelMesh;
}

use render::components::*;

make_bundle!(RenderBundle, |w|{}, Camera, Mesh);