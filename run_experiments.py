#!/usr/bin/env python3
import argparse
import os
import subprocess
import sys

def main():
    parser = argparse.ArgumentParser(
        description="Run the solver executable on a directory of instances.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Example usage:
  python3 run_experiments.py debug instances/baptiste-2008/loose-equal --searchMethod LS --neighborhood SWAP_ADJ
        """
    )
    
    parser.add_argument("build_type", choices=["debug", "release"], 
                        help="The build type to use (determines the executable path).")
    parser.add_argument("instances_dir", 
                        help="Directory containing the problem instances.")
    parser.add_argument("solver_args", nargs=argparse.REMAINDER, 
                        help="Additional arguments to pass to the solver (e.g., --seed 10).")

    args = parser.parse_args()

    # Determine executable path based on build type
    base_dir = os.getcwd()
    if args.build_type == 'debug':
        exe_path = os.path.join(base_dir, 'debug_buid/programs/solver/solver')
    else:
        exe_path = os.path.join(base_dir, 'release_build/programs/solver/solver')

    # Verify executable exists
    if not os.path.isfile(exe_path):
        print(f"\033[91mError: Executable not found at: {exe_path}\033[0m")
        print(f"Make sure you have built the {args.build_type} version of the project.")
        sys.exit(1)

    # Verify instances directory exists
    if not os.path.isdir(args.instances_dir):
        print(f"\033[91mError: Instances directory not found: {args.instances_dir}\033[0m")
        sys.exit(1)

    # Collect all instance files (assuming .txt extension based on project structure)
    instances = []
    for root, dirs, files in os.walk(args.instances_dir):
        for file in files:
            if file.endswith(".txt"):
                instances.append(os.path.join(root, file))

    instances.sort()

    if not instances:
        print(f"No .txt instance files found in {args.instances_dir}")
        sys.exit(0)

    # Execute solver for each instance
    for i, instance_path in enumerate(instances):
        cmd = [exe_path, instance_path] + args.solver_args
        
        try:
            # Run the command and capture output
            result = subprocess.run(cmd, capture_output=True, text=True)
            
            if result.returncode == 0:
                # Print only the stdout from the executable
                print(result.stdout.strip())
            else:
                # Print error to stderr if needed
                print(result.stderr, file=sys.stderr)
                
        except KeyboardInterrupt:
            sys.exit(1)
        except Exception as e:
            print(f"Error executing command: {e}", file=sys.stderr)

if __name__ == "__main__":
    main()
