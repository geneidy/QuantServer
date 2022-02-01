# QuantServer: High Frequency Market Data Infrastructure Server

High performance multi-threaded *Limit Order Book* (re)constructor using raw data feed messages adhering to the Nasdaq ITCH 5.0 protocol specification. Intended for high frequency trading research such as trade cost analysis. This has not been tested in a live environment.  

## Build

make:
```sh
make all
```

clean:
```sh
make clean
```

## Run
command line arguments can be found in `main.cpp`.

Sample data is publicly available through <ftp://emi.nasdaq.com/ITCH/>. Please be aware this is partial data sampled throughout the trading day to test parser implementation.  


