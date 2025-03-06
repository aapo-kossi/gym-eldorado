{{ name | escape | underline }}

.. currentmodule:: {{ module }}

.. autoclass:: {{ fullname }}
   :members:
   :undoc-members:
   :show-inheritance:

{% if methods %}
   **Methods:**

   {% for item in methods %}
   - :meth:`{{ item.fullname }}`
   {% endfor %}
{% endif %}

{% if attributes %}
   **Attributes:**

   {% for item in attributes %}
   - :attr:`{{ item.fullname }}`
   {% endfor %}
{% endif %}

