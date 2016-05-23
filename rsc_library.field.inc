<?php


// Fields defined by this module, that do not change based on user settings:
define('RSC_LIBRARY_FIELDS_STATIC', array(
  array( // DEPRECATED. TODO: migrate content from the body field to summary
    'field_name'  => RSC_LIBRARY_PREFIX.'_body',
    'type'        => 'text_long',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_summary',
    'type'        => 'text_long',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_full_text',
    'type'        => 'text_long',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_attachment',
    'type'        => 'file',
    'cardinality' => 1,
    'settings'    => array(
      'uri_scheme' => 'public',
    ),
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_pages',
    'type'        => 'number_integer',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_weight',
    'type'        => 'number_integer',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_format',
    'type'        => 'list_text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_editor',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_code',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_publisher',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_link',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_chapter',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_publisher',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_buy',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_date',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_book_title',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_page_range',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_copyright_notice',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_featured_image',
    'type'        => 'image',
    'cardinality' => 1,
    'settings'    => array(
      'uri_scheme' => 'public',
    ),
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_featured',
    'type'        => 'list_boolean',
    'cardinality' => 1,
    'settings'    => array(
      'allowed_values' => array(
        0 => '',
        1 => '',
      ),
    ),
  ),
  array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_sort_enabled',
    'type'        => 'list_boolean',
    'cardinality' => 1,
    'settings'    => array(
      'allowed_values' => array(
        0 => '',
        1 => '',
      ),
    ),
  ),
));


/**
 * (re)create fields
 * FIXME: this uses outdated $settings variable if function is called in a library node edit form submit handler.
 * Workaround: submit form once for setting vocabs, second time for creating fields
 */
function rsc_library_field__setup_fields() {

  // body text with summary for library front page
  node_add_body_field(node_type_get_type('rsc_library'));

  // get a list of existing fields, so that we don't add duplicates:
  $field_names = array_keys(field_info_fields());

  // add fields that do not change based on user settings:
  foreach (RSC_LIBRARY_FIELDS_STATIC as $field_info) {
    $field_name = $field_info['field_name'];
    if (!in_array($field_name, $field_names)) {
      field_create_field($field_info);
    }
  }

  // Create taxonomy reference fields for each library:
  foreach(rsc_library_get_lids() as $lid) {
    rsc_library_field__setup_taxonomy_fields_for_library($lid);
  }

}


/**
 * Create "taxonomy term reference" fields for a specific library
 */
function rsc_library_field__setup_taxonomy_fields_for_library($lid) {

  // cardinality settings for the taxonomy fields added below
  static $cardinality = array(
    'author'     => FIELD_CARDINALITY_UNLIMITED,
    'editor'     => FIELD_CARDINALITY_UNLIMITED,
    'tag'        => FIELD_CARDINALITY_UNLIMITED,
    'category'   => FIELD_CARDINALITY_UNLIMITED,
    'difficulty' => 1,
    'source'     => 1,
  );

  // Get the proper translation function, since this function might run during install, or any other time
  $t = get_t();

  // Check if the library settings are configured properly
  $library_settings = variable_get("rsc_library_{$lid}", NULL);
  if (empty($library_settings['vocabs'])) {
    drupal_set_message($t('Could not create fields for library @lid, because its vocabulary settings have not been configured yet.', array('@lid' => $lid)), 'warning', FALSE);
    return;
  }
  foreach($library_settings['vocabs'] as $field_short_name => $vid) {

    $error = FALSE;

    // check if a vocabulary has been configured
    if (!($vid > 0)) {
      drupal_set_message($t('Could not create @field field for library @lid, because a vocabulary has not been selected yet.', array(
        '@field' => $field_short_name,
        '@lid' => $lid,
      )), 'warning', FALSE);
      $error = TRUE;
    }

    if ($error) return;

  }

  // get a list of existing fields, so that we don't add duplicates:
  $field_names = array_keys(field_info_fields());

  // for each taxonomy reference field
  foreach ($library_settings['vocabs'] as $field => $vid) {

    // load the vocabulary info
    $vocab = taxonomy_vocabulary_load($vid);

    // determine what this field should be called
    $field_name = RSC_LIBRARY_PREFIX . "_{$field}_{$vocab->machine_name}";
    if (strlen($field_name) > 32) {
      drupal_set_message(t('Cannot create field @fieldname (more than 32 characters).', array('@fieldname' => $field_name)), 'error');
      continue;
    }

    // create it if it doesn't exist yet
    if (!in_array($field_name, $field_names)) {
      field_create_field(array(
        'field_name' => $field_name,
        'type' => 'taxonomy_term_reference',
        'cardinality' => $cardinality[$field],
        'settings' => array(
          'allowed_values' => array(
            array(
              'vocabulary' => $vocab->machine_name,
              'parent' => '0',
            ),
          ),
        ),
      ));

      // add field name to list to prevent duplicate creation attempts
      $field_names[] = $field_name;
    }
  }

}


/**
 * (re)configure field instances
 */
function rsc_library_field__setup_field_instances() {

  // Get the proper translation function, since this function might run during install, or any other time
  $t = get_t();

  // Partially define the field instances for nodes
  // TODO: add a short description for each field instance
  $field_instances_on_nodes = array(
    'body' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_body',
      'label'       => $t('Summary'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'label' => 'hidden',
        ),
      ),
    ),
    'attachment' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_attachment',
      'label'       => $t('Attachment'),
      'required'    => false,
      'settings'    => array(
        'file_extensions' => 'pdf',
        'file_directory'  => 'articles',
      ),
      'display'     => array(
        'default' => array(
          'label' => 'hidden',
        ),
      ),
    ),
    'link' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link',
      'label'       => $t('Link'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'link_to_chapter' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_chapter',
      'label'       => $t('Link to chapter'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'link_to_publisher' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_publisher',
      'label'       => $t('Link to publisher'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'link_to_buy' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_buy',
      'label'       => $t('Link to buy'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'pages' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_pages",
      'label'       => $t('Pages'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'weight' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_weight",
      'label'       => $t('Weight'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'format' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_format",
      'label'       => $t('Format'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'editor' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_editor",
      'label'       => $t('Editor'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'code' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_code",
      'label'       => $t('Code'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'publisher' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_publisher",
      'label'       => $t('Publisher'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'date' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_date",
      'label'       => $t('Date'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'book_title' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_book_title",
      'label'       => $t('Book title'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'page_range' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_page_range",
      'label'       => $t('Page range'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'copyright_notice' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_copyright_notice",
      'label'       => $t('Copyright notice'),
      'description' => $t('Displayed as: This material is made available with the permission of ___.'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ),
    'featured_image' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_featured_image",
      'label'       => $t('Featured image'),
      'required'    => false,
      'settings'    => array(
        'file_directory'  => 'featured_images',
        'file_extensions' => 'png gif jpg jpeg',
        'alt_field'       => 1,
        'title_field'     => 0,
      ),
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type'     => 'image_image',
        'settings' => array(
          // TODO: depend on manualcrop module and define those settings here?
          /*
          'manualcrop_enable' => 1,
          'manualcrop_keyboard' => 1,
          'manualcrop_thumblist' => 1,
          'manualcrop_inline_crop' => 0,
          'manualcrop_crop_info' => 1,
          'manualcrop_instant_preview' => 1,
          'manualcrop_instant_crop' => 1,
          'manualcrop_default_crop_area' => 1,
          'manualcrop_maximize_default_crop_area' => 1,
          'manualcrop_styles_mode' => 'include',
          'manualcrop_styles_list' => array(
            'sidebar_thumbnail' => 'sidebar_thumbnail'
          ),
          'manualcrop_require_cropping' => array(
            'sidebar_thumbnail' => 'sidebar_thumbnail'
          ),
          */
          // TODO: depend on filefield_aources module and define those settings here?
          /*
          'filefield_sources' => array(
            'filefield_sources' => array(),
            'source_reference' => array(
              'autocomplete' => '0'
            ),
            'source_attach' => array(
              'path' => 'file_attach',
              'absolute' => 0,
              'attach_mode' => 'move',
            ),
            'source_imce' => array(
              'imce_mode' => 0,
            ),
          ),
          */
        ),
      ),
    ),

  );

  // define the field instances for each content type, because not all content
  // types use all the fields
  $field_instances_by_type = array(
    'rsc_library_article' => array(
      $field_instances_on_nodes['body'], // TODO: migrate the content of this field to "summary" and remove this field instance
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['full_text'],
      $field_instances_on_nodes['attachment'],
      $field_instances_on_nodes['link'], // TODO: move external articles to own content type, and remove this field instance
      $field_instances_on_nodes['pages'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['format'],
      $field_instances_on_nodes['editor'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['publisher'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['featured_image'],
    ),
    'rsc_library_link' => array(
      $field_instances_on_nodes['body'],
      $field_instances_on_nodes['link'],
      $field_instances_on_nodes['pages'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['format'],
      $field_instances_on_nodes['editor'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['publisher'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['featured_image'],
    ),
    'rsc_library_request' => array(
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['book_title'],
      $field_instances_on_nodes['publisher'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['page_range'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['featured_image'],
    ),
    'rsc_library_book' => array(
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['book_title'],
      $field_instances_on_nodes['publisher'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['pages'],
      $field_instances_on_nodes['copyright_notice'],
      $field_instances_on_nodes['attachment'],
      $field_instances_on_nodes['link_to_chapter'],
      $field_instances_on_nodes['link_to_publisher'],
      $field_instances_on_nodes['link_to_buy'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['featured_image'],
    ),
  );

  // add the field instances to the content types
  foreach ($field_instances_by_type as $type => $field_instances) {
    rsc_library_field__create_field_instances_on_bundle('node', $type, $field_instances);
  }

  // add per-library field instances to various bundles
  foreach(rsc_library_get_lids() as $lid) {
    rsc_library_field__setup_field_instances_for_library($lid);
  }

}


/**
 * Create taxonomy reference field instances on the rsc_library type,
 * for the specified library, depending on the library settings.
 */
function rsc_library_field__setup_field_instances_for_library($lid) {

  // Get the proper translation function, since this function might run during install, or any other time
  $t = get_t();

  // Check if the library settings are configured properly
  $library_settings = variable_get("rsc_library_{$lid}", NULL);
  if (empty($library_settings['vocabs'])) {
    drupal_set_message($t('Could not create fields for library @lid, because its vocabulary settings have not been configured yet.', array('@lid' => $lid)), 'warning', FALSE);
    return;
  }
  foreach($library_settings['vocabs'] as $field_short_name => $vid) {

    $error = FALSE;

    // check if a vocabulary has been configured
    if (!($vid > 0)) {
      drupal_set_message($t('Could not create @field field for library @lid, because a vocabulary has not been selected yet.', array(
        '@field' => $field_short_name,
        '@lid' => $lid,
      )), 'warning', FALSE);
      $error = TRUE;
    }

    if ($error) return;

  }


  /*
   * FIELD INSTANCES ON NODE TYPES
   *
   * Create taxonomy reference fields for category, author, source, tag and
   * difficulty on the various content types defined by this module, based on
   * the settings for this library:
   */

  // Load the library front page node to get the library title
  $node_library_frontpage = node_load($lid);
  $library_title = $node_library_frontpage->title;

  // partially define the field instances
  $taxonomy_reference_field_instances = array(
    'author' => array(
      'label'       => $t('Author'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array('type' => 'taxonomy_autocomplete'),
    ),
    'editor' => array(
      'label'       => $t('Editor'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array('type' => 'taxonomy_autocomplete'),
    ),
    'source' => array(
      'label'       => $t('Source'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array('type' => 'taxonomy_autocomplete'),
    ),
    'tag' => array(
      'label'       => $t('Tags'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array('type' => 'taxonomy_autocomplete'),
    ),
    'category' => array(
      'label'       => $t('Category (in "@lib")', array('@lib'=>$library_title)),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type'     => 'taxonomy_shs',
        'settings' => array(
          'shs' => array(
            'node_count' => TRUE,
            'create_new_terms' => FALSE,
            'create_new_levels' => FALSE,
            'force_deepest' => FALSE,
          ),
        ),
      ),
    ),
    'difficulty' => array(
      'label'       => $t('Difficulty'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array('type' => 'options_buttons'),
    ),
  );

  // for each taxonomy reference field used by this library
  foreach($library_settings['vocabs'] as $field_short_name => $vid) {

    // load the vocabulary
    $vocab = taxonomy_vocabulary_load($vid);

    // determine what this field is called
    $field_name = RSC_LIBRARY_PREFIX . "_{$field_short_name}_{$vocab->machine_name}";
    $taxonomy_reference_field_instances[$field_short_name]['field_name'] = $field_name;

  }

  // Make a list of field instances for each content type, because not all
  // content types use all fields.
  $field_instances_by_type = array(
    'rsc_library_article' => array(
      $taxonomy_reference_field_instances['author'],
      $taxonomy_reference_field_instances['source'],
      $taxonomy_reference_field_instances['tag'],
      $taxonomy_reference_field_instances['category'],
      $taxonomy_reference_field_instances['difficulty'],
    ),
    'rsc_library_link' => array(
      $taxonomy_reference_field_instances['author'],
      $taxonomy_reference_field_instances['source'],
      $taxonomy_reference_field_instances['tag'],
      $taxonomy_reference_field_instances['category'],
      $taxonomy_reference_field_instances['difficulty'],
    ),
    'rsc_library_request' => array(
      $taxonomy_reference_field_instances['author'],
      $taxonomy_reference_field_instances['editor'],
      $taxonomy_reference_field_instances['tag'],
      $taxonomy_reference_field_instances['category'],
      $taxonomy_reference_field_instances['difficulty'],
    ),
    'rsc_library_book' => array(
      $taxonomy_reference_field_instances['author'],
      $taxonomy_reference_field_instances['editor'],
      $taxonomy_reference_field_instances['tag'],
      $taxonomy_reference_field_instances['category'],
      $taxonomy_reference_field_instances['difficulty'],
    ),
  );

  // add the field instances to the content types
  foreach ($field_instances_by_type as $type => $field_instances) {
    rsc_library_field__create_field_instances_on_bundle('node', $type, $field_instances);
  }


  /*
   * FIELD INSTANCES ON TAXONOMY TERMS
   *
   * Create "featured" and "enable sort" field instances on all taxonomy terms
   * belonging to a vocabulary that is being used as categories for a library.
   */

  // Partially define the field instances
  $field_instances_on_taxonomy_terms = array(
    array(
      'field_name'  => RSC_LIBRARY_PREFIX."_featured",
      'label'       => $t('Featured'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type'     => 'options_onoff',
        'settings' => array(
          'display_label' => 1,
        ),
      ),
    ),
    array(
      'field_name'  => RSC_LIBRARY_PREFIX."_sort_enabled",
      'label'       => $t('Enable node sorting'),
      'required'    => false,
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type'     => 'options_onoff',
        'settings' => array(
          'display_label' => 1,
        ),
      ),
    ),
  );

  // Get a list of field instances using this library's category vocabulary
  $category_vocabulary = taxonomy_vocabulary_load($library_settings['vocabs']['category']);

  // Create the field instances on the appropriate bundle
  rsc_library_field__create_field_instances_on_bundle('taxonomy_term', $category_vocabulary->machine_name, $field_instances_on_taxonomy_terms);

}


/**
 * Delete unused fields that start with RSC_LIBRARY_PREFIX_
 * Unused means: does not have any instances
 */
function rsc_library_field__cleanup() {
  $fields = array_keys(field_info_fields());
  $used_fields = array_keys(field_info_field_map());
  $unused_fields = array_diff($fields,$used_fields);

  $n = strlen(RSC_LIBRARY_PREFIX) + 1;

  foreach ($unused_fields as $field) {
    if (substr($field, 0, $n) === RSC_LIBRARY_PREFIX.'_') {
      field_delete_field($field);
    }
  }
}


/**
 * Create field instances on a bundle if they don't exist yet.
 */
function rsc_library_field__create_field_instances_on_bundle($entity_type, $bundle, $new_field_instances) {

  // Get a list of existing fields and field instances
  $existing_fields = array_keys(field_info_fields());
  $existing_field_instances = array_keys(field_info_instances('node','rsc_library_article'));

  // Create each field instance if it does not exist yet
  foreach ($new_field_instances as $new_field_instance) {
    $field_name = $new_field_instance['field_name'];
    if (
      !in_array($field_name, $existing_field_instances) &&  // is not an existing field instance
      in_array($field_name, $existing_fields)  // is an existing field
    ) {
      $new_field_instance['entity_type'] = $entity_type;
      $new_field_instance['bundle'] = $bundle;
      field_create_instance($new_field_instance);
      $existing_field_instances[] = $field_name;
    }
  }

}

