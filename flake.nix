{
  description = "C/C++ environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, ... }@inputs:
    utils.lib.eachDefaultSystem (
      system:
      let
        p = import nixpkgs { 
				inherit system;
				config = { 
					allowUnfree = true; 
				}; 
			};
        llvm = p.llvmPackages_latest;
      in
      {
        devShell = p.mkShell.override { stdenv = p.clangStdenv; } rec {
          packages = with p; [
			# builder
			cmake

			# debugger
			gdb

			# fix headers not found
			clang-tools

			# LSP and compiler
			llvm.libstdcxxClang

			# other tools
			llvm.libllvm

			# stdlib for cpp
			llvm.libcxx
				
			# libs
			libGL
			glfw
			mesa.dev
          ];
          name = "C";
        };
      }
    );
}