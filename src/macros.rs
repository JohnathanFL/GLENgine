macro_rules! make_bundle {
    ($modname: ident, $afterFunc:expr, $($compName: ident),*) => {
        pub struct $modname;

        impl Bundle for $modname {
            fn add_to_world(self, world: &mut World) {

                $(
                    world.register::<$compName>();
                )*


                $afterFunc(world);

            }
        }
    }


}