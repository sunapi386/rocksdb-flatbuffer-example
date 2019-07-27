Example how to use Flatbuffer with RocksDB. The program write/read key values. Keys are strings and values are Flatbuffer byte array of 92 bytes.

# Install

- Install rocksdb, flatbuffer. 
- `flatc --cpp game.fbs` to get the `game_generated.h`.

# Test Setup

- ASUS Series ROG Zephyrus S Model GX531GX-XS74
- Intel Core i7-8750H 2.20 GHz
- 16 GB DDR4 (8 GB x 2)
- 512 GB PCIe NVMe SSD
- GeForce RTX 2080 Max-Q 8 GB GDDR6
- Ubuntu 16.04 LTS 64-bit


| K-V Objects|   Time (ms) |    CPU (%)  |   Real (MB)  | Virtual (MB)  | Disk Size |
| -----------|-------------|-------------|--------------|---------------|------------- |
| 1          |      0.000  |      0.000  |      0.777   |     4.398     |     152K |
| 10         |      0.889  |      0.000  |      0.660   |     4.398     |     156K |
| 100        |      2.438  |      0.000  |      0.762   |     4.398     |     168K |
| 1000       |     25.359  |      0.000  |      0.762   |     4.398     |     280K |
| 10000      |    278.974  |      0.000  |     22.598   |   361.199     |     1.5M |
| 100000     |   2892.830  |    101.100  |     33.578   |   432.402     |     13M |
| 1000000    |  29171.700  |     98.700  |     98.348   |   552.551     |     115M |

# References

- `psrecord --include-children --log $prog_name.log --interval 0.5 --plot plot-$prog_name.png $argv`
- https://unix.stackexchange.com/questions/554/how-to-monitor-cpu-memory-usage-of-a-single-process
