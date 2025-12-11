from pathlib import Path

def parse_cgroup_info(cgroup_path):
    """Parse cgroup information from sysfs."""
    info = {}
    
    # Read memory limits if available
    memory_high = cgroup_path / "memory.high"
    if memory_high.exists():
        try:
            info['memory.high'] = int(memory_high.read_text().strip())
        except ValueError:
            info['memory.high'] = "max"
            
    memory_max = cgroup_path / "memory.max"
    if memory_max.exists():
        try:
            info['memory.max'] = int(memory_max.read_text().strip())
        except ValueError:
            info['memory.max'] = "max"
            
    # Read CPU limits if available
    cpu_weight = cgroup_path / "cpu.weight"
    if cpu_weight.exists():
        info['cpu.weight'] = cpu_weight.read_text().strip()
        
    # Read current usage
    memory_current = cgroup_path / "memory.current"
    if memory_current.exists():
        info['memory.current'] = int(memory_current.read_text().strip())
        
    return info

def generate_cgroup_tree():
    """Generate a mermaid diagram of the cgroup hierarchy."""
    cgroup_root = Path("/sys/fs/cgroup")
    if not cgroup_root.exists():
        print("Cgroup filesystem not found")
        return
        
    # Start dot diagram
    print("digraph cgroup_tree {")
    print("    rankdir=TB;")
    print("    node [shape=box, style=filled, fillcolor=lightblue];")
    
    def traverse_cgroup(path, parent_id=None):
        current_id = path.name or "root"
        info = parse_cgroup_info(path)
        
        # Create node label with limits
        label = f"{current_id}"
        if info:
            limits = []
            if 'memory.max' in info:
                limits.append(f"mem: {info['memory.max']}")
            if 'cpu.weight' in info:
                limits.append(f"cpu: {info['cpu.weight']}")
            if limits:
                label += f"\\n({', '.join(limits)})"
                
        print(f'    "{current_id}" [label="{label}"];')
        
        if parent_id:
            print(f'    "{parent_id}" -> "{current_id}";')
            
        # Recursively process subdirectories
        for subdir in path.iterdir():
            if subdir.is_dir() and not subdir.name.startswith('.'):
                traverse_cgroup(subdir, current_id)
    
    traverse_cgroup(cgroup_root)
    print("}")

if __name__ == "__main__":
    generate_cgroup_tree()
