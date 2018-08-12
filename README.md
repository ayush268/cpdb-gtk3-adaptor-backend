# GTK3 Adaptor Backend for Common Print Dialog

This repository contains one of the components I worked on as part of my Google Summer of Code'18 project with the Linux Foundation, the complete documentation can be found [here](https://github.com/ayush268/GSoC_2018_Documentation).

This repository hosts the code for the GTK3 Adaptor Backend for the **C**ommon **P**rint **D**ialog. This backend will act as a mediator between the GTK3 dialog (frontend) and other CPDB backends like CUPS, GCP etc.

```
GTK3 dialog  <-> GTK3 dialog backend <-> the CPD Frontend <-> the CPDB backends
```

## Running

Currently the backend is not complete, a few api implementations are remaining.

## Background

The [Common Printing Dialog](https://wiki.ubuntu.com/CommonPrintingDialog) project aims to provide a uniform, GUI toolkit independent printing experience on Linux Desktop Environments.
