---
title: SCIRun Module Documentation
category: info
tags: module
layout: modules_toc
---

<script async
  src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=TeX-AMS_CHTML">
</script>


<link rel="stylesheet" href="css/main.css">
<link rel="stylesheet" href="css/modules.css">

<script>
    function toggle_visibility(id) {
       var e = document.getElementsByName(id)[0];
       if(e.style.display == 'block')
          e.style.display = 'none';
       else
          e.style.display = 'block';
    }

     window.onload = function() {
        if(window.location.href.indexOf("#") > -1) {
          toggle_visibility("id_" + window.location.hash.substring(1))[0];
         }
    }
</script>

<style>
div.hidden {
    display: none;
}
</style>

<a id="top"></a>

# SCIRun Modules

{% comment %}from https://gist.github.com/pepelsbey/9334494{% endcomment %}
{% capture tmp %}
  {% for page in site.pages %}
    {% if page.category == "moduledocs" %}
      {{ page.module.category }}
    {% endif %}
  {% endfor %}
{% endcapture %}

{% assign categories = tmp | split: ' ' %}
{% assign tmp = categories[0] %}

{% for cat in categories %}
  {% unless tmp contains cat %}
    {% capture tmp %}{{ tmp }} {{ cat }}{% endcapture %}
  {% endunless %}
{% endfor %}

{% assign modulecategories = tmp | split: ' ' %}

{% capture modulepages %}
  {% for cat in modulecategories %}
    ?{{ cat }}
    {% for page in site.pages %}
      {% if page.module.category == cat %}
        ${{ page.title }}#{{ page.url }}
      {% endif %}
    {% endfor %}
  {% endfor %}
{% endcapture %}

{% assign sortedpages = modulepages | strip | strip_newlines | split: '?' | sort %}

{% for pagestring in sortedpages %}
  {% assign pageitems = pagestring | split: '$' %}
  <div class="content" markdown="1">
  {% if pageitems[0] %}
  <header class="title" markdown="1"><h2 id="{{ pageitems[0] | strip }}">{{ pageitems[0] }}</h2></header>
    {% for item in pageitems %}

      {% comment %}skip category list item (index 0){% endcomment %}
      {% if forloop.first %} {% continue %} {% endif %}
      {% assign linkitem = item | split: '#' %}
      {% assign contentId = linkitem[0] | prepend: 'id_' %}
<div class="more-content" markdown="1">
<div class="sub-heading" markdown="1">
  <h5>
    <a name="{{linkitem[0]}}" data-proofer-ignore></a>
    <a onclick="toggle_visibility('{{ contentId }}');" style="cursor: pointer;" data-proofer-ignore> {{ linkitem[0] }} </a>
    <!-- <script>
        if(window.location.href.indexOf("#") > -1) {
          if('{{ contentId }}'.localeCompare("id_"+ window.location.hash.substring(1)) == 0) {
              toggle_visibility('{{ contentId }}');
          }
        }
    </script> -->

  </h5>
</div>
      {% capture mdpath %}modules/{{linkitem[0]}}.md{% endcapture %}
      {% capture my-include %}{% include {{mdpath}} %}  {% endcapture %}
      {% assign importantPart1 = my-include | split: 'Summary' %}
      {% assign importantPart2 = importantPart1[1] %}
<div class="hidden" markdown="1" name="{{contentId}}">{{ importantPart2  }} </div>
</div>

    {% endfor %}
  {% endif %}
  </div>
  ---
{% endfor %}
