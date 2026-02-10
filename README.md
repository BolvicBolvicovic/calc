# Calc Interpreter

A calculator for my journey as an embedded system engineer.
Manipulate physics units and orders of magnetude with ease.

![Julia set with c = -0.8i](./mkdocs/docs/julia_set_c-0.8i.png)

## Installation

This is very straight forward. Clone the repository and run make.

```bash
git clone git@github.com:BolvicBolvicovic/calc.git
cd calc
make
```

It will compile and launch the interpreter by default.
The only requirements are the libmath and libreadline which should be installed by default and a gcc compiler.

## Usage

I have written an extensive documentation [here](./mkdocs/docs/documentation.md).
Note that you can build and read it from your browser at [localhost:8000/documentation](http://localhost:8000/documentation).

```bash
make docs_serve
```

## Testing

Run `make tests`.
It will create and run the `tester` binary. If a test fails, the `assert` macro displays a message.
Else, nothing is printed.

Run `make bench` to run the benchmark. It does a zoom in the burning ship fractal from 2 to 1e-13.
The zoom decays by log 10 every time a boundry is reached. Note that it writes the result to `/dev/null`.
If you want to vizualize it, run the program `benchmark` after compilation.
Current `perf stat`:

```
 Performance counter stats for './benchmark':

    14,820,572,427      task-clock:u                     #    1.000 CPUs utilized
                 0      context-switches:u               #    0.000 /sec
                 0      cpu-migrations:u                 #    0.000 /sec
               334      page-faults:u                    #   22.536 /sec
   163,556,562,496      instructions:u                   #    2.82  insn per cycle
                                                         #    0.04  stalled cycles per insn
    57,975,025,197      cycles:u                         #    3.912 GHz
     6,442,925,966      stalled-cycles-frontend:u        #   11.11% frontend cycles idle
    34,459,924,901      branches:u                       #    2.325 G/sec
        23,960,873      branch-misses:u                  #    0.07% of all branches

      14.823855195 seconds time elapsed

      14.774783000 seconds user
       0.046996000 seconds sys
```

## Upcomming features

Upcomming features and ideas are documented [here](./mkdocs/docs/index.md).
You can also access it from your browser at [localhost:8000](http://localhost:8000).
