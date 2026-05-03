import os
import subprocess
import shutil
from datetime import datetime

jobs = [50]
machines = [10, 20]
due_dates = ["LOOSE", "TIGHT"]
weights = ["EQUAL", "TARD"]
replications = [1, 2]

base_dir = "instances/generated"
easy_instances_dir = "instances/solved_by_cplex"
solutions_file = os.path.join(easy_instances_dir, "solutions.txt")
exe_path = "./release_build/programs/instance-generator/instance-generator"
solver_exe = "./release_build/programs/solver/solver"
max_milli = 3600000  # 1 hour (3,600,000 ms)
timeout_buffer = 60  # 1 minute buffer for Python timeout

# Check if executables exist to avoid runtime errors
# ... (rest of the check)

# Ensure base dir exists
os.makedirs(base_dir, exist_ok=True)
os.makedirs(easy_instances_dir, exist_ok=True)

for dd in due_dates:
    for w in weights:
        dir_name = f"{dd.lower()}-{w.lower()}"
        full_dir_path = os.path.join(base_dir, dir_name)
        os.makedirs(full_dir_path, exist_ok=True)
        
        for j in jobs:
            for m in machines:
                # Set time limit to 1 hour
                current_max_milli = max_milli
                for r in replications:
                    # Construct filename
                    filename = f"test{r}_{j}x{m}.txt"
                    filepath = os.path.join(full_dir_path, filename)
                    
                    if os.path.exists(filepath):
                        print(f"Skipping {filepath} (already exists)")
                        continue

                    # Construct command
                    cmd = [
                        exe_path,
                        "--j", str(j),
                        "--m", str(m),
                        "--dd", dd,
                        "--w", w
                    ]
                    
                    while True:
                        print(f"Generating candidate for {filepath}...")
                        try:
                            with open(filepath, "w") as outfile:
                                # Generator should be fast, but we add a timeout just in case
                                subprocess.run(cmd, stdout=outfile, check=True, timeout=300) 
                        except subprocess.CalledProcessError as e:
                            print(f"Generator failed: {e}")
                            break # Go to next replication/config
                        except subprocess.TimeoutExpired:
                            print(f"Generator timed out for {filepath}. Retrying...")
                            continue

                        # Test with solver
                        solver_cmd = [
                            solver_exe,
                            "--instPath", filepath,
                            "--solveExact",
                            "--maxMilli", str(current_max_milli),
                            "--autoConfig"
                        ]
                        
                        start_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                        print(f"[{start_time}] Testing {filepath} with CPLEX (limit {current_max_milli}ms)...")
                        try:
                            # Run solver with a hard timeout in Python (current_max_milli + buffer)
                            result = subprocess.run(
                                solver_cmd, 
                                capture_output=True, 
                                text=True, 
                                check=True, 
                                timeout=(current_max_milli / 1000) + timeout_buffer
                            )
                            output = result.stdout
                            
                            # Check status
                            if "CPLEX Status: Optimal" in output:
                                print("Status: Optimal. Saving to easy instances and regenerating...")

                                # Parse objective value (last non-empty line with autoConfig)
                                lines = [l.strip() for l in output.strip().splitlines() if l.strip()]
                                obj_val = lines[-1] if lines else "UNKNOWN"

                                # Generate unique filename in easy_instances_dir
                                base_name = os.path.basename(filepath)
                                name_root, ext = os.path.splitext(base_name)
                                
                                counter = 1
                                while True:
                                    candidate_name = f"{name_root}_{counter}{ext}"
                                    candidate_path = os.path.join(easy_instances_dir, candidate_name)
                                    if not os.path.exists(candidate_path):
                                        break
                                    counter += 1
                                
                                # Copy file
                                shutil.copy(filepath, candidate_path)
                                
                                # Append to solutions file
                                with open(solutions_file, "a") as f:
                                    f.write(f"{candidate_name} {obj_val}\n")
                                    
                                print(f"Saved easy instance to {candidate_path} with obj {obj_val}")
                                
                                continue  # Retry (regenerate)
                            else:
                                # Look for status line to print
                                status_line = "Unknown"
                                for line in output.splitlines():
                                    if "CPLEX Status:" in line:
                                        status_line = line.strip()
                                        break
                                print(f"Accepted. {status_line}")
                                break  # Accepted
                        
                        except subprocess.TimeoutExpired:
                            print(f"[{datetime.now().strftime('%H:%M:%S')}] Solver exceeded hard timeout (1h). Terminating process.")
                            # When timeout occurs, the process is killed. We accept the instance as "hard"
                            break

                        except subprocess.CalledProcessError as e:
                            print(f"Solver execution failed (likely error): {e}")
                            if e.stdout and "CPLEX Status:" in e.stdout:
                                print(f"Solver output status: {e.stdout.splitlines()[-1]}")
                            
                            print("Retrying generation...")
                            continue

print("Done.")
