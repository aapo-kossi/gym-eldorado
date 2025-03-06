{{ name | escape | underline }}

.. currentmodule:: {{ module }}

.. autoattribute:: {{ fullname }}

{% if value %}
   **Value:** {{ value }}
{% endif %}

