use std::sync::Arc;

use vulkano::{
    self as vk,
    instance::QueueFamily,
    device::Device,
    command_buffer::AutoCommandBufferBuilder,
    buffer::{CpuAccessibleBuffer, BufferUsage},
};

use volume::Voxel;
use math::*;


#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct VertPos {
    pub pos: (f32, f32, f32),
}

#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct VertData {
    pub data: Voxel,
}

impl_vertex!(VertPos, pos);
impl_vertex!(VertVoxel, data);

impl From<Voxel> for VertVoxel {
    #[inline]
    fn from(data: Voxel) -> Self {
        return VertVoxel { data };
    }
}


impl From<Vec3> for VertPos {
    #[inline]
    fn from(x: Vec3) -> Self {
        return VertPos {
            pos: x.into()
        };
    }
}

#[derive(Debug, Clone)]
pub struct VoxelMesh {
    dirty: bool,
    pub positions: Vec<VertPos>,
    pub voxels: Vec<VertVoxel>,

    posBuff: Option<Arc<CpuAccessibleBuffer<[VertPos]>>>,
    voxBuff: Option<Arc<CpuAccessibleBuffer<[VertVoxel]>>>,
}

impl VoxelMesh {
    pub fn new() -> VoxelMesh {
        return VoxelMesh {
            /// Does the GPU buffered mesh need to be updated?
            dirty: true,
            positions: Vec::new(),
            voxels: Vec::new(),
            posBuff: None,
            voxBuff: None,
        };
    }

    #[inline]
    pub fn mark_dirty(&mut self) {
        self.dirty = true;
    }

    #[inline]
    pub fn reserve(&mut self, n: usize) {
        self.positions.reserve(n);
        self.voxels.reserve(n);
    }

    #[inline]
    pub fn push(&mut self, vertPack: (VertPos, VertVoxel)) {
        self.mark_dirty();

        let (pos, voxel) = vertPack;

        self.positions.push(pos);
        self.voxels.push(voxel);
    }

    #[inline]
    pub fn shrink_to_fit(&mut self) {
        self.positions.shrink_to_fit();
        self.voxels.shrink_to_fit();
    }

    #[inline]
    pub fn upload(&mut self, device: &Arc<Device>, transferCmdBuff: &mut AutoCommandBufferBuilder) {
        self.posBuff = Some(CpuAccessibleBuffer::from_iter(device.clone(), BufferUsage::all(),
                                                      self.positions.iter().cloned()).unwrap());

        self.voxBuff = Some(CpuAccessibleBuffer::from_iter(device.clone(), BufferUsage::all(),
                                                           self.voxels.iter().cloned()).unwrap());
    }

    #[inline]
    pub fn get_buffs(&self) -> Result<(Arc<CpuAccessibleBuffer<[VertPos]>>,
    Arc<CpuAccessibleBuffer<[VertVoxel]>>), ()> {
        let posBuff = match self.posBuff {
            Some(ref buff) => buff.clone(),
            None => return Err(())
        };

        let voxBuff = match self.voxBuff {
            Some(ref buff) => buff.clone(),
            None => return Err(())
        };

        return Ok((posBuff, voxBuff));
    }
}