# Report selected Github Insights

Pulls insight information from your repositories on github.  
Creates report in either TXT or HTML format.

These fields are reported:
* Number of clones during the past 14 days.
* Number of forks
* Number of watchers
* Number of stars

Data from each run is saved in a local file name <owner>.CSV.  
If counts change in a subsequent run, the changes are flagged with asterisks.  
As days pass, the number of clones might decrease; therefore clones are printed as <previous>/<current>[*]

```
USAGE: GitInsights owner {txt|html}
```

