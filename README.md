# AAD-JITJS: Automated Algorithm Design for the Just-in-Time Job Shop Scheduling Problem

This project explores the application of **Automated Algorithm Design (AAD)** to solve a challenging variant of the job shop scheduling problem: the **Just-in-Time Job Shop Scheduling Problem (JIT-JSSP)**.

## Contextualization

### Automated Algorithm Design (AAD)

Traditionally, the development of algorithms for combinatorial optimization problems involves a manual trial-and-error process to select components (such as neighborhoods and acceptance criteria) and tune parameters. **Automated Algorithm Design** replaces this manual effort with systematic and automated techniques.

In this project, we use the **irace** tool to automatically configure the solver, selecting the best combination of search heuristics (Local Search, ILS, Tabu Search), dispatching rules, and neighborhood strategies for different types of problem instances.

### Just-in-Time Job Shop Scheduling Problem (JIT-JSSP)

The **Job Shop Scheduling Problem (JSSP)** is one of the most well-known scheduling problems, where a set of jobs must be processed on a set of machines following a specific order.

In the **Just-in-Time (JIT)** variant, the objective is not just to complete jobs as quickly as possible (minimize makespan), but rather to complete them as close as possible to their **due dates**. This implies penalties for both:

- **Tardiness:** When a job finishes after its due date, incurring costs due to customer dissatisfaction or penalties.
- **Earliness:** When a job finishes before its due date, incurring inventory holding costs and unnecessary space occupation.

The challenge of JIT-JSSP lies in the delicate balance between these two penalties, making it a highly complex optimization problem with significant industrial relevance.

## Project Overview

This project is composed of two main programs:

1. **Instance Generator (`programs/instance-generator`):** This program is responsible for creating various instances of the Just-in-Time Job Shop Scheduling Problem. These instances serve as test cases for evaluating the performance of different algorithmic configurations.

2. **Solver Framework (`programs/solver`):** This is the core framework for solving the JIT-JSSP. It is designed with a modular architecture, allowing for the integration and selection of diverse algorithmic components for different stages of the search process. These components include:
    - **Initial Solution Alternatives:** Different methods to construct a starting solution.
    - **Dispatching Rules:** Rules to decide the order of operations.
    - **Neighborhoods:** Operators to explore the solution space.
    - **Search Heuristics:** High-level strategies like Local Search, Iterated Local Search (ILS), and Tabu Search.
    - **Neighborhood Traversing Strategies:** Methods to move within a neighborhood (e.g., Best Improvement, First Improvement).
    - **Acceptance Criteria:** Rules for accepting new solutions during the search.
    - **Scheduling Alternatives:** Methods to schedule operations.
    - **Perturbations:** Techniques to escape local optima in metaheuristics like ILS.

## Usage

### Prerequisites

To build and run the project, you need:

- **IBM ILOG CPLEX Optimization Studio:** Required for the solver's exact method and certain scheduling components.
- **R (version >= 3.2.0):** Required for automated algorithm design (tuning).
- **irace package:** The R package for automated configuration.

### Compilation

This project uses CMake for building. Ensure you have CMake installed.
To compile the project, navigate to the root directory and follow these steps:

```bash
mkdir build
cd build
cmake ..
make
```

This will create `instance-generator` and `solver` executables in the `build/programs/instance-generator` and `build/programs/solver` directories respectively.

### Running the Programs

#### 1. Instance Generator (`instance-generator`)

This program generates problem instances according to specified parameters, following the methodology described in Baptiste et al. (2008).

**Usage:**

```bash
./build/programs/instance-generator/instance-generator [OPTIONS]
```

**Parameters:**

- `-j <unsigned>`: Number of jobs.
- `-m <unsigned>`: Number of machines.
- `--dd <string>`: Due date type. Influences the tightness of due dates. Accepts `TIGHT` or `LOOSE`. ([Baptiste et al., 2008](#references))
- `-w <string>`: Penalties type. Determines the relative weight of earliness and tardiness penalties. Accepts `EQUAL` (equal earliness and tardiness penalties) or `TARD` (tardiness penalties are significantly higher than earliness penalties, penalizing late completion more severely). ([Baptiste et al., 2008](#references))

**Example:**

```bash
./build/programs/instance-generator/instance-generator -j 10 -m 5 --dd TIGHT -w EQUAL > instances/generated/test_10x5_tight_equal.txt
```

#### 2. Solver Framework (`solver`)

This program executes the JIT-JSSP solver with various configurable algorithmic components.

**Usage:**

```bash
./build/programs/solver/solver <instPath> [OPTIONS]
```

**Parameters:**

- `<instPath>`: **(Positional argument)** Path to the instance file to solve.
- `--maxMilli <unsigned>`: Maximum execution time in milliseconds (default: `60000`).
- `--seed <unsigned>`: Seed for the random number generator (default: `13`).
- `--initSol <string>`: Initial solution algorithm. Accepts `GT` (Giffler-Thompson) or `CONSTR` (Constructive by available operations) (default: `GT`).
- `--dispatchRule <string>`: Dispatching rule for initial solutions. Accepts `EDD`, `ACS`, or `RAND` (default: `EDD`).
- `--search1 <string>`: Main search method. Accepts `LS` (Local Search), `ILS` (Iterated Local Search), or `TABU` (Tabu Search) (default: `LS`).
- `--search2 <string>`: Internal search method if ILS is chosen for `--search1`. Accepts `LS` or `TABU` (default: `LS`).
- `--numNHoods <unsigned>`: Number of neighborhoods to use (from 1 to 7) (default: `1`).
- `--nhood[1-7] <string>`: Neighborhood structure. Accepts `SWAP_ADJ`, `SWAP_RAND`, `INSERT_RAND`, `SWAP_PENAL`, `INSERT_PENAL`, `CRITICAL_OPER`, `CRITICAL_OPER_ALT` (default: `SWAP_ADJ` for each).
- `--nHoodTravers[1-7] <string>`: Neighborhood traversing strategy. Accepts `BI` (Best Improvement), `FI` (First Improvement), or `ELT_LIST` (Elite List based) (default: `BI` for each).
- `--sched <string>`: Scheduler type. Accepts `EARLY`, `CPLEX`, `DELAYING`, or `HYB` (default: `EARLY`).
- `--solveExact`: (Flag) If present, solves using the CPLEX exact method (overrides other search methods).
- `--autoConfig`: (Flag) If present, prints only the objective value for automatic configuration tools (e.g., irace).
- `--maxD <unsigned>`: Maximum size of repeated sequence during Tabu Search (default: `5`).
- `--maxC <unsigned>`: Maximum number of repetitions of the same cycle during Tabu Search (default: `3`).
- `--tenure <unsigned>`: Number of tabu iterations a move remains tabu (default: `20`).
- `--jumpListSz <unsigned>`: Number of past elite states to store for backjumping (default: `20`).
- `--initJumpLimit <unsigned>`: Number of iterations without improvement to trigger a backjump (default: `10`).
- `--decreaseDivisor <unsigned>`: Factor by which `jumpLimit` is reduced after each jump (default: `2`).
- `--perturbationStrength <unsigned>`: Perturbation strength for ILS (1-100) (default: `30`).
- `--ilsIterMaxMilli <unsigned>`: Maximum time in milliseconds for a single ILS iteration (default: `1000000000`).

**Example:**

```bash
./build/programs/solver/solver instances/baptiste-2008/loose-equal/test1_10x10.txt --maxMilli 10000 --initSol GT --dispatchRule EDD --search1 ILS --perturbationStrength 50 --numNHoods 2 --nhood1 SWAP_RAND --nHoodTravers1 FI
```

## Automated Algorithm Design with irace

The **irace** package is used to automatically find the best configurations for the solver.

### Installation

To install the **irace** package, open an R terminal and run:

```R
install.packages("irace")
```

### Running the Calibration

To start the automated design process, use the provided R script from the root directory:

```bash
Rscript tuning/run_irace.R
```

The script will read the configuration from `tuning/scenario.txt` and `tuning/parameters.txt`, using the instances listed in `tuning/train-instances.txt`.

## Acknowledgments

This project uses the following third-party components:

- **IBM ILOG CPLEX Optimization Studio:** High-performance mathematical programming solver. Visit [the IBM website](https://www.ibm.com/products/ilog-cplex-optimization-studio).
- **PCG Random Number Generation:** Developed by Melissa O'Neill. Visit [http://www.pcg-random.org](http://www.pcg-random.org).
- **irace (Iterative Racing):** Developed by Manuel López-Ibáñez, Jérémie Dubois-Lacoste, Leslie Pérez Cáceres, Thomas Stützle, and Mauro Birattari. For more information and citations, visit [the irace website](http://iridia.ulb.ac.be/irace/).

## <a name="references"></a> References

- Baptiste, P., Flamini, M., & Sourd, F. (2008). Lagrangian bounds for just-in-time job-shop scheduling. *Computers & Operations Research*, *35*(3), 906-915.
