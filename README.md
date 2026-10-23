# Load Balancer Simulation

**Brice Ockman - CSCE (Load Balancer Project), C++17**

A discrete-time simulation of a company load balancer distributing web
requests across a pool of web servers. Requests carry a source IP, a
destination IP, a processing time in clock cycles, and a job type
(`P` = processing, `S` = streaming).

Features:

- Initial queue of `servers x 100` requests, with new requests arriving at
  random times during the run
- Dynamic scaling: servers are added when the queue grows above
  `80 x servers` and removed when it falls below `50 x servers`, with a
  configurable cooldown between scaling actions (draining servers finish
  their current job before being deallocated)
- Firewall / IP range blocker (dash ranges and CIDR) driven by the config file
- Colored console output, verbose and quiet modes, and a plain-text log file
- Bonus: a `Switch` that routes `P` and `S` jobs to two separate load
  balancers running concurrently, each with its own pool and statistics

## Build

```sh
make            # builds ./loadbalancer
make test       # builds and runs the unit tests
make docs       # generates Doxygen HTML in docs/html (requires doxygen)
make clean
```

## Run

```sh
./loadbalancer --config config/default.cfg --verbose
./loadbalancer --config config/default.cfg --quiet --servers 20 --cycles 100000
./loadbalancer --config config/switch.cfg --switch
```

With no `--servers` / `--cycles` flags the program prompts for them; pressing
Enter accepts the defaults from the config file. Required deliverable run:
20 servers, 100,000 cycles (log in `logs/`).

## Layout

```
include/  Request, RequestQueue, WebServer, LoadBalancer, Firewall,
          RequestGenerator, Config, Logger, Colors, Stats, Switch headers
src/      matching .cpp files plus main.cpp (the driver)
config/   default.cfg, switch.cfg
tests/    assert-based unit tests (make test)
```

All settings (task times, arrival rate, scaling band and cooldown, blocked
IP ranges, seed, verbose/quiet, colors, log file) live in the config file -
see `config/default.cfg` for the full list. A fixed `seed` makes runs
reproducible.
