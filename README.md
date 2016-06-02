# rsc_library
Drupal module for managing RSC's online libraries in Drupal

## What it does:

### rsc_library.admin.inc

- Define the module settings page
- Define a form for the node settings page

### rsc_library.blocks.inc

- Define three blocks:
 - Featured nodes
 - New nodes
 - Featured categories
- Define another "teaser block" for each library node

### rsc_library.common.inc

- Define content types
- Define content type settings
- Function to get all library node IDs

### rsc_library.field.inc

- Define fields and field instances
- Functions to create and update fields and field instacnes

### rsc_library.install

- Set content type settings
- Create fields and instances
- Create user roles
- Grant permissions on those roles

### rsc_library.module

- Define permissions
- Define menu callbacks
- Define content types
- Set breadcrumbs for the site
- Determine which library is being used (influences block display, etc.)
- Define a view mode for nodes shown in thie module's blocks
- TODO: check node access
- When saving a node, automatically populate the rscl_format field to reflect the media types used in the node
- Register templates

### rsc_library.template.inc

- Manipulate the way fields are rendered on nodes types defined by this module
- Add microdata

### /templates/

- Field templates (`field--*.tpl.php`)
- Node templates (`node--*.tpl.php`)
