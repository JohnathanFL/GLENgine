#![allow(non_snake_case)]
#![feature(pattern_parentheses)]
#![allow(unused_imports)]
#![allow(dead_code)]
#![feature(custom_attribute)]

#![feature(drain_filter)]
#![feature(const_str_as_ptr)]

extern crate smallvec;

extern crate num_traits;

#[macro_use]
extern crate vulkano;
extern crate winit;
use winit::{
    WindowEvent,
    Window,
    EventsLoop,
    dpi::LogicalSize,
    Event,
    KeyboardInput,
    VirtualKeyCode
};

pub extern crate cgmath;

pub extern crate image;
extern crate specs;
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
mod chunk;
mod volume;
mod scene;
mod render;
mod components;

use render::Renderer;

struct Keybindings {
    quit: usize,
    yaw: usize,
}

struct App {
    pub events: EventsLoop,
    pub renderer: Renderer,
}

#[derive(Ord, PartialOrd, Eq, PartialEq)]
enum AppStatus {
    Continue, Exit, Error(i32)
}

impl AppStatus {
    fn transition(&mut self, newStatus: AppStatus) {
        // Only transition if the new status is of greater severity
        if newStatus > *self {
            *self = newStatus;
        }
    }
}

impl App {
    pub fn new() -> App {
        let (events, renderer) = Renderer::new();
        
        App {
            events,
            renderer
        }
    }

    pub fn update(&mut self) -> AppStatus {
        let mut status = AppStatus::Continue;

        self.events.poll_events(|ev| {
            if let Event::WindowEvent{event,..} = ev {
                match event {
                    WindowEvent::CloseRequested => status.transition(AppStatus::Exit),
                    WindowEvent::KeyboardInput {input, ..} => {
                        if let Some(keycode) = input.virtual_keycode {
                            if keycode == VirtualKeyCode::Escape {
                                status.transition(AppStatus::Exit);
                            }
                        }
                    },

                    _ => {}
                }
            }
        });


        return status;
    }

    pub fn window(&self) -> &Window {
        self.renderer.get_window()
    }
}


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
