Title: How to Profile Yarn App/Container Memory Usage
Date: 2015-01-13
Category: Technology
Tags: Big Data

Yarn does not provide a tool to profile the memory usage of an app yet, but it does save some instrumentation information to the log. Like this.

```
yarn-wdong-nodemanager-washtenaw.log:2015-01-06 14:56:43,267 INFO org.apache.hadoop.yarn.server.nodemanager.containermanager.monitor.ContainersMonitorImpl: Memory usage of ProcessTree 16669 for container-id container_1420574192658_0001_01_000001: 277.3 MB of 9 GB physical memory used; 8.9 GB of 18.9 GB virtual memory used
```

The numbers reported are actually those based on which Yarn kills processes.

[This script](https://github.com/aaalgo/yarn-memory-tracker) analyzes the log and reports maximal memory usage of each container for a particular app.

Sample output

```
$ yarn-memory-tracker.sh application_1421176927536_0002    # an spark app
383 containers found for app application_1421176927536_0002
container_1421176927536_0001_01_000001: 0.254785 of 16.4 GB
container_1421176927536_0001_01_000002: 16.2 of 51.4 GB
container_1421176927536_0001_01_000003: 0.00107422 of 51.4 GB
container_1421176927536_0001_01_000004: 0.00107422 of 51.4 GB
container_1421176927536_0001_01_000005: 12.5 of 51.4 GB
container_1421176927536_0002_01_000001: 0.251563 of 16.4 GB
container_1421176927536_0002_01_000002: 16.1 of 51.4 GB
......

```
