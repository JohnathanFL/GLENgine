extern crate proc_macro;


extern crate proc_macro2;

use proc_macro2::Span;

extern crate syn;
use syn::{Data, DeriveInput, Fields, DataStruct, FieldsUnnamed, Ident};


#[macro_use]
extern crate quote;

use quote::{TokenStreamExt, ToTokens};

use proc_macro::TokenStream;


#[proc_macro_derive(NewType)]
pub fn derive_new_type(input: TokenStream) -> TokenStream {
    let ast: DeriveInput = syn::parse(input).unwrap();

    let name: &Ident = &ast.ident;
    let modname = Ident::new(&format!("newtypeimpl_{}", name.to_string()), Span::call_site());
    let fields = match ast.data {
        syn::Data::Struct(dataStruct) => {
            match dataStruct.fields {
                syn::Fields::Unnamed(fields) => {
                    fields.unnamed
                },
                _ => panic!("#[derive(DerefAsInner)] MUST be used on a tuple struct with a \
                    single value!")
            }
        },

        _ => panic!("#[derive(DerefAsInner)] MUST be used on a tuple struct")
    };
    let innertype = fields.first().unwrap();

    let quoted = quote! {
        pub mod #modname {
            use super::*;
            use std::ops::{Deref, DerefMut};
            impl Deref for #name {
                type Target = #innertype;
                fn deref(&self) -> &Self::Target {
                    return &self.0;
                }
            }

            impl DerefMut for #name {
                fn deref_mut(&mut self) -> &mut Self::Target {
                    return &mut self.0;
                }
            }

            impl From<#innertype> for #name {
                fn from(x: #innertype) -> Self {
                    return #name(x);
                }
            }

            impl From<#name> for #innertype {
                fn from(x: #name) -> Self {
                    return x.0;
                }
            }
        }
        pub use self::#modname::*;

    };

    quoted.into()
}
