#!/usr/bin/env bash
set -e # halt script on error

bundle exec jekyll build
bundle exec htmlproofer ./_site --allow-hash-href --url_swap \/scirun.pages:
