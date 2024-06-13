### Environment & setup
```bash
git clone https://github.com/cmstas/NanoTools
cd NanoTools/
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /cvmfs/cms.cern.ch/slc7_amd64_gcc900/cms/cmssw/CMSSW_11_2_0_pre5/ ; cmsenv ; cd -
cd NanoCORE
make test -j12
cd ..
```

### Unit tests

Unit tests of `NanoCORE/` are inside `NanoCORE/Tools/unit_tests/` and can be run with
`make test` inside of `NanoCORE/`.

### Skimmer

One must long on to ```slc7``` machines (e.g. ```uaf-1/7/8/10``` for this.  
The newer linux version won't work. (e.g. ```uaf-2/3```)  
For future we will have to one day fix this as ```slc7``` will be gone.  

```bash
ssh uaf-10.t2.ucsd.edu
mkdir -p workarea/skimtest
cd workarea/skimtest
git clone https://github.com/cmstas/NanoTools
cd NanoTools/
source /cvmfs/cms.cern.ch/cmsset_default.sh
cd /cvmfs/cms.cern.ch/slc7_amd64_gcc900/cms/cmssw/CMSSW_11_2_0_pre5/ ; cmsenv ; cd -
cd NanoCORE
make test -j12
cd ..
cd skimmer/
make -j;
./skim -h # To print help information
# Testing one job (-d option defines where the output directory will be. below example sets it to current directory)
./skim \
    -d ./ \
    /ceph/cms/store/user/phchang/nanoclass/mc/RunIISummer20UL18NanoAODv9/TTToSemiLeptonic_TuneCP5_13TeV-powheg-pythia8/NANOAODSIM/106X_upgrade2018_realistic_v16_L1v1-v1/120000/87DEE912-70CF-A549-B10B-1A229B256E88.root
cd condor/ProjectMetis
source setup.sh
cd ../
sh maketar.sh # create tarball to submit to condor jobs
# Setup voms proxy (certificate)
voms-proxy-init -hours 168 -voms cms -rfc
# Submit jobs to condor
python submit.py
```

### Style

We use `clang-format` based on LLVM style to format our code. To format the `ElectronSelections.cc` file in-place, do
```bash
clang-format -style="{BasedOnStyle: llvm, IndentWidth: 4, ColumnLimit: 120, AllowShortIfStatementsOnASingleLine: true, AllowShortBlocksOnASingleLine: true}" -i ElectronSelections.cc
```

Add this to the ```~/.vimrc``` and use ```vim``` to code. (Assumes you are working on UAF)
```
autocmd BufNewFile,BufRead *.cc,*.h,*.C,*.cxx set formatprg=clang-format\ -style=\"{BasedOnStyle:\ llvm,\ IndentWidth:\ 4,\ ColumnLimit:\ 100,\ AllowShortIfStatementsOnASingleLine:\ true,\ AllowShortBlocksOnASingleLine:\ false,\ BreakBeforeBraces:\ Allman}\"
```

To format your code, press ```ggvGgq```.


### Grid certificate installation

Copy the certificate to a file named myCert.p12 to the computer where you will run voms-proxy-init.

Extract your certificate (which contains the public key) and the private key:

Extract the certificate:
openssl pkcs12 -in myCert.p12 -clcerts -nokeys -out $HOME/.globus/usercert.pem
Extract the encrypted private key:
openssl pkcs12 -in myCert.p12 -nocerts -out $HOME/.globus/userkey.pem
You must set the mode on your userkey.pem file to read/write only by the owner, otherwise voms-proxy-init will not use it:
chmod 600 $HOME/.globus/userkey.pem
chmod 600 $HOME/.globus/usercert.pem
Delete the myCert.p12 file created above to avoid security issues.

NOTE: Additionally, see https://ca.cern.ch/ca/ for more information
