@{
    ExcludeRules = @(
        # build.ps1 uses Write-Host intentionally for colored terminal output
        'PSAvoidUsingWriteHost',
        # Script-scope parameters used inside function bodies trigger false positives
        'PSReviewUnusedParameter'
    )
}
