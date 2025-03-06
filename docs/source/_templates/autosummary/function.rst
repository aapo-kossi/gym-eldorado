{{ name | escape | underline }}

.. currentmodule:: {{ module }}

.. autofunction:: {{ fullname }}

{% if members %}
   **Parameters:**

   {% for param in members.params %}
   - **{{ param.name }}** ({{ param.annotation }}): {{ param.description }}
   {% endfor %}
{% endif %}

{% if return %}
   **Returns:**
   {{ return.annotation }} – {{ return.description }}
{% endif %}

