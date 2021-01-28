# iDMAnalysis
iDM Analysis repository (skimming, trigger/reco efficiency, and analysis)

## Environment setup

```bash
# For all years:
$ cmsrel CMSSW_10_2_18
$ cd CMSSW_10_2_18/src
$ cmsenv
```

## Pull and compile packages

Pull KalmanVertexFit and VertexTools packages to extend the native Kalman Filter range limit:

```bash
$ git cms-addpkg RecoVertex/VertexTools
$ git cms-addpkg RecoVertex/KalmanVertexFit
```
and apply the following git patch:

```bash
$ curl https://home.fnal.gov/~as2872/extendTrackerDimensions.patch | git apply
```

Now clone the iDM repo (either via SSH or HTTPS if you don't have SSH keys) and compile everything:

```bash
$ git clone git@github.com:afrankenthal/iDMAnalysis.git # or https://github.com/afrankenthal/iDMAnalysis.git
$ scram b -j 8
$ cd iDMAnalysis/skimmer
```

## Running ntuplizer to make flat trees from AODs:

Might need to renew your proxy first:

```bash
$ voms-proxy-init -voms cms -valid 192:00
```

For a local test run:

```bash
$ cmsRun python/iDMAnalyzer_cfg.py test=1
```

Call multicrab to run over more samples in EOS:

```bash
$ cd python
$ ./multicrab_iDMAnalyzer -c submit -w WORKAREA -s X # where X = {data,MC,custom}
```
