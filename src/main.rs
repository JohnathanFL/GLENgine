#![allow(non_snake_case)]
#![feature(pattern_parentheses)]
#![allow(unused_imports)]
#![allow(dead_code)]
#![feature(custom_attribute)]
#![allow(unused_parens)]

#![feature(drain_filter)]
#![feature(const_str_as_ptr)]

#[macro_use]
#[macro_reexport]
extern crate glengine_derive;

extern crate smallvec;

extern crate num_traits;

#[macro_use]
extern crate vulkano;
#[macro_use]
extern crate vulkano_shader_derive;

extern crate winit;

use winit::{
    WindowEvent,
    Window,
    EventsLoop,
    dpi::LogicalSize,
    Event,
    KeyboardInput,
    VirtualKeyCode,
};

pub extern crate cgmath;

pub extern crate image;
extern crate specs;

use specs::{System, World, DispatcherBuilder, Dispatcher, AsyncDispatcher, Builder};

#[macro_use]
extern crate specs_derive;


extern crate bidrag;

use bidrag::{Key, MouseButton, MouseAxis, Binding};

use std::any::{Any, TypeId};
use std::rc::Rc;
use std::sync::Arc;
use std::collections::HashMap;
use std::ptr::null;
use std::default::Default;
use std::ffi::CString;
use std::os::raw::c_char;
use std::process::exit;
use std::sync::mpsc::Receiver;


static APP_NAME: &str = "GLENgine-rs Test";
static ENGINE_NAME: &str = "GLENgine-rs";

mod vulkano_win;

mod math;

use math::*;

mod chunk;
mod volume;
mod scene;

use scene::{NodeBuilder, hierarchy::{Node, RootNode}, transform::Transformation};

mod mesh;
mod render;
use render::{self, Renderer};
mod components;
mod defaultpipeline;


fn main() {
    let mut app = App::new();

    'main: loop {
        let status = app.update();
        match status {
            AppStatus::Continue => continue,
            AppStatus::Exit => break 'main,
            AppStatus::Error(code) => exit(code)
        }
    }
}


struct Keybindings {
    quit: usize,
    yaw: usize,
}

struct App<'a> {
    pub window: Arc<vulkano::swapchain::Surface<Window>>,
    pub events: EventsLoop,
    pub world: World,
    pub dispatcher: Dispatcher<'a, 'a>,
    pub rootNode: specs::Entity
}

#[derive(Ord, PartialOrd, Eq, PartialEq)]
enum AppStatus {
    Continue,
    Exit,
    Error(i32),
}

impl AppStatus {
    fn transition(&mut self, newStatus: AppStatus) {
        // Only transition if the new status is of greater severity
        if newStatus > *self {
            *self = newStatus;
        }
    }
}

impl<'a> App<'a> {
    pub fn new() -> App<'a> {
        println!("Initializing renderer...");
        let (events, renderer) = Renderer::new();
        println!("Renderer intialized!");

        let window = renderer.get_window();

        let mut world = {
            let mut world = World::new();
            world.add_bundle(scene::SceneBundle);
            world.add_bundle(render::RenderBundle);
            world.add_bundle(volume::VolumeBundle);

            world
        };
        let rootNode = **world.read_resource::<scene::hierarchy::RootNode>();

        let mut vol = volume::Volume::new(volume::DrawStyle::ChunkByChunk);

        {
            vol.add_chunk((0, 0, 0).into());
            let chunk = vol.chunk_mut((0, 0, 0).into()).unwrap();
            chunk.set(vec3(0, 0, 0), 1);
        }

        let camNode = world.new_node(Some(rootNode),
                                     Transformation::at(
                                         vec3(0.0, 0.0, -1.0),
                                         Quat::from_angle_x(Deg(0.0)),
                                     ),
        )
            .with(render::components::Camera::new(0.into(), 90.0, 0.001, 1000.0))
            .build();

        let volNode = world.new_node(Some(rootNode), Transformation::origin())
            .with(vol)
            .build();

        let dispatchBuilder = DispatcherBuilder::new()
            .with(scene::transform::TransformCalc, "transform_calc", &[])
            .with(renderer, "renderer", &["transform_calc"]);

        dispatchBuilder.print_par_seq();

        let dispatcher = dispatchBuilder.build();

        App {
            window,
            events,
            world,
            dispatcher,
        }
    }

    pub fn update(&mut self) -> AppStatus {
        let mut status = AppStatus::Continue;

        self.events.poll_events(|ev| {
            if let Event::WindowEvent { event, .. } = ev {
                match event {
                    WindowEvent::CloseRequested => status.transition(AppStatus::Exit),
                    WindowEvent::KeyboardInput { input, .. } => {
                        if let Some(keycode) = input.virtual_keycode {
                            if keycode == VirtualKeyCode::Escape {
                                status.transition(AppStatus::Exit);
                            }
                        }
                    }

                    _ => {}
                }
            }
        });
        self.dispatcher.dispatch(&mut self.world.res);

        return status;
    }

    pub fn window(&self) -> &Window {
        return self.window.window();
    }
}


