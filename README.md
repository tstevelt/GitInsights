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

My daily report as of May 12, 2024 <a href='https://www.silverhammersoftware.com/GitInsights05122024.png'>Click here</a>

```
USAGE: GitInsights owner {txt|html}
```

