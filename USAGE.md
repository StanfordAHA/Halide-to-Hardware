# RDAI Usage Instructions
[What is RDAI?](https://github.com/thenextged/rdai/blob/master/README.md)

## Installation
Clone the `rdai` repo directly outside the halide-to-hardware compiler's top directory (where `clockwork`, `coreir`, and `BufferMapping` dependencies are expected by default).

RDAI repo is located [here](https://github.com/thenextged/rdai.git)

For a full halide-to-hardware compiler setup on Kiwi (including RDAI), please browse [here](INSTALL.md).

## Applications
Applications are split into two folders in `Halide-to-Hardware/apps/hardware_benchmarks`:
- `tests` features simpler applications like conv_3_3.
- `apps` features more complex applications like resnet.

## Running the Pipeline
In order to run a full Halide-to-Hardware pipeline, follow these steps:
1. From within the halide-to-hardware compiler's top directory, change into the application directory: `cd apps/hardware_benchmarks/path/to/app/`
2. In the app's Makefile, you can choose the RDAI C-Simulation platform runtime by changing `RDAI_PLATFORM_RUNTIME` to `clockwork_sim`. Available platform runtimes are located in `rdai/platform_runtimes` under the `rdai` repo. Similarly, you can choose a different host runtime by changing/adding the variable `RDAI_HOST_RUNTIME` in the app's Makefile. Available RDAI host runtimes are located at `rdai/host_runtimes` under the `rdai` repo. The default `RDAI_PLATFORM_RUNTIME` and `RDAI_HOST_RUNTIME` setup allows a C-Simulation of the Halide pipeline.
3. Generate target design: `make compiler && make <TARGET>` where `TARGET` can be `cpu`, `clockwork`, `coreir`, etc.
4. Run pipeline with implemented hardware kernels: `make run-<TARGET>` (e.g. `make run-clockwork`)
5. Compare output to cpu output: `make compare-<TARGET>` (e.g. `make compare-clockwork`)

Here is a list of the different make targets:
<pre><code>make clean               # remove generated files (bin directory)
     compiler            # compile updates to Halide compiler
     generator           # create Halide generator
     cpu                 # create CPU design
     clockwork           # create clockwork design
     image               # create an image with random data
     run-cpu             # create output file using CPU implementation
     run-clockwork       # create output file using clockwork implementation
     compare-clockwork   # compare Clockwork output file to CPU output image
     eval                # evaluate runtime </code></pre>

The definition of all of these targets can be found in `apps/hardware_benchmarks/hw_support/hardware_targets.mk`.

## Halide-to-Hardware Directory Tree for Apps:
<pre><code>Halide-to-Hardware
└── apps
    └── hardware_benchmarks                    // contains simpler test cases
        ├── apps                               // contains all apps compiled to coreir
        └── tests                              // contains all simpler test cases
            └── conv_3_3                       // one of the apps: does 3x3 convolution
                ├── Makefile                   // specifies commands for 'make'
                ├── conv_3_3_generator.cpp     // contains Halide algorithm and schedule
                ├── input.png                  // input image for testing
                ├── process.cpp                // runs input image with design to create ouput image
                ├── golden                     // this holds all expected output files
                │   └── golden_output.png      // output image expected
                │
                └── bin                        //// Running 'make clockwork' generates in this folder:
                    ├── output_cpu.png         // output image created using CPU implementation
                    └── output_clockwork.png   // output image created during testing; should be equivalent to output_cpu.png</code></pre>
