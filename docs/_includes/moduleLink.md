{% if page.url contains '/modules.html' %}[{{include.moduleName}}](#{{include.moduleName}}){% else %}
[{{include.moduleName}}](../modules.html#{{include.moduleName}}){% endif %}
