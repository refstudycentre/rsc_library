<?php

require_once "rsc_library.common.inc";


/**
 * (re)create fields
 * FIXME: this uses outdated $settings variable if function is called in a library node edit form submit handler.
 * Workaround: submit form once for setting vocabs, second time for creating fields
 */
function rsc_library_field__setup_fields() {

  // body text with summary for library front page
  node_add_body_field(node_type_get_type('rsc_library'));

  // Create static fields (i.e. those that do not change based on library settings)
  rsc_library_field__setup_static_fields();

  // Create taxonomy reference fields for each library:
  foreach(rsc_library_get_lids() as $lid) {
    rsc_library_field__setup_taxonomy_fields_for_library($lid);
  }

}


/**
 * @throws \Exception
 * @throws \FieldException
 */
function rsc_library_field__setup_static_fields() {

  // Get the proper translation function, since this function might run during install, or any other time
  $t = get_t();

  // Fields defined by this module, that do not change based on library settings:
  $rsc_library_fields_static = array(
    array( // DEPRECATED. TODO: migrate content from the body field to summary
      'field_name' => RSC_LIBRARY_PREFIX . '_body',
      'type' => 'text_long',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_summary',
      'type' => 'text_long',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_full_text',
      'type' => 'text_long',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_attachment',
      'type' => 'file',
      'cardinality' => -1,
      'settings' => array(
        'uri_scheme' => 'public',
      ),
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_pages',
      'type' => 'number_integer',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_weight',
      'type' => 'number_integer',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_format',
      'type' => 'list_text',
      'cardinality' => -1,
      'settings' => array(
        'allowed_values' => array(
          'text' => $t('Text'),
          'audio' => $t('Audio'),
          'video' => $t('Video'),
        ),
      ),
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_code',
      'type' => 'text',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_link',
      'type' => 'text',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_link_to_chapter',
      'type' => 'link_field',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_link_to_publisher',
      'type' => 'link_field',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_link_to_buy',
      'type' => 'link_field',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_date',
      'type' => 'text',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_book_title',
      'type' => 'text',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_page_range',
      'type' => 'text',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_copyright_notice',
      'type' => 'text',
      'cardinality' => 1,
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_featured_image',
      'type' => 'image',
      'cardinality' => 1,
      'settings' => array(
        'uri_scheme' => 'public',
      ),
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_featured',
      'type' => 'list_boolean',
      'cardinality' => 1,
      'settings' => array(
        'allowed_values' => array(
          0 => '',
          1 => '',
        ),
      ),
    ),
    array(
      'field_name' => RSC_LIBRARY_PREFIX . '_sort_enabled',
      'type' => 'list_boolean',
      'cardinality' => 1,
      'settings' => array(
        'allowed_values' => array(
          0 => '',
          1 => '',
        ),
      ),
    ),
  );

  // get a list of existing fields, so that we don't add duplicates:
  $field_names = array_keys(field_info_fields());

  // add fields that do not change based on library settings:
  foreach ($rsc_library_fields_static as $field_info) {
    $field_name = $field_info['field_name'];
    if (in_array($field_name, $field_names)) {
      field_update_field($field_info);
    } else {
      field_create_field($field_info);
    }
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
    'publisher'  => 1,
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
  foreach ($library_settings['vocabs'] as $field_shortname => $vid) {

    // load the vocabulary info
    $vocabulary = taxonomy_vocabulary_load($vid);

    // determine what this field should be called
    $field_name = RSC_LIBRARY_PREFIX . "_{$field_shortname}_{$vocabulary->machine_name}";
    if (strlen($field_name) > 32) {
      drupal_set_message(t('Cannot create field @fieldname (more than 32 characters).', array('@fieldname' => $field_name)), 'error');
      continue;
    }

    $field = array(
      'field_name' => $field_name,
      'type' => 'taxonomy_term_reference',
      'cardinality' => $cardinality[$field_shortname],
      'settings' => array(
        'allowed_values' => array(
          array(
            'vocabulary' => $vocabulary->machine_name,
            'parent' => '0',
          ),
        ),
      ),
    );

    if (in_array($field_name, $field_names)) {
      // update the existing field info
      field_update_field($field);
    } else {
      // create it if it doesn't exist yet
      field_create_field($field);
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
  $field_instances_on_nodes = array(
    'summary' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_summary',
      'label'       => $t('Summary'),
      'description' => $t('What is this node about? This field is displayed in lists, e.g. search results.'),
      'required'    => FALSE,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_default',
          'weight' => 0,
        ),
        'teaser' => array(
          'label' => 'hidden',
          'type' => 'text_trimmed',
          'weight' => 2,
          'settings' => array(
            'trim_length' => 600,
          ),
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 2,
      ),
    ),
    'code' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_code",
      'label'       => $t('Code'),
      'description' => $t('A unique identifier according to RSC\'s catalogue of web content.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'type' => 'hidden',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 3,
      ),
    ),
    'book_title' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_book_title",
      'label'       => $t('Book title'),
      'description' => $t('Title of the book to which this chapter belongs.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_default',
          'weight' => 4,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 6,
      ),
    ),
    'date' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_date",
      'label'       => $t('Date'),
      'description' => $t('The year of publication.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_default',
          'weight' => 7,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 8,
      ),
    ),
    'pages' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_pages",
      'label'       => $t('Pages'),
      'description' => $t('The number of pages.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'number_integer',
          'weight' => 8,
          'settings' => array(
            'thousand_separator' => ' ',
            'decimal_separator' => '.',
            'scale' => 0,
            'prefix_suffix' => FALSE,
          ),
          'module' => 'number',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 10,
      ),
    ),
    'page_range' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_page_range",
      'label'       => $t('Page range'),
      'description' => $t('The range of page numbers, e.g. 13–27'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_default',
          'weight'   => 9,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 11,
      ),
    ),
    'format' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_format",
      'label'       => $t('Format'),
      'description' => $t('The media types used in this content.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'type' => 'hidden',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 12,
        'type'   => 'options_buttons',
      ),
    ),
    'copyright_notice' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_copyright_notice",
      'label'       => $t('Copyright notice'),
      'description' => $t('Displayed as: This material is made available with the permission of ___.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_plain',
          'weight' => 15,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 13,
      ),
    ),
    'attachment' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_attachment',
      'label'       => $t('Attachment'),
      'description' => $t('PDFs, MP3s, etc. containing media for this node.'),
      'required'    => FALSE,
//      'settings'    => array( // This must differ for each bundle, to lower the number of files per directory
//        'file_extensions' => 'pdf',
//        'file_directory'  => 'articles',
//      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden', // TODO: rsc_attachment_formatter
//          'type' => 'rsc_attachment_formatter_formatter_dropdown',
          'weight' => 10,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 15,
      ),
    ),
    'link' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link',
      'label'       => $t('Link'),
      'description' => $t('Link to an external website. Include the full anchor tag, like this: !html', array(
        '!html' => htmlspecialchars('<a href="" target="_blank">Read article</a>'),
      )),
      'required'    => FALSE,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_default',
          'weight' => 11,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 16,
      ),
    ),
    'link_to_chapter' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_chapter',
      'label'       => $t('Link to chapter'),
      'description' => $t('Link to an external website containing the full text for the chapter. The "title" is the text which will appear on the button.'),
      'required'    => FALSE,
      'settings'    => array(
        'absolute_url'    => 1,
        'validate_url'    => 1,
        'url'             => 0,
        'title'           => 'required',
        'title_maxlength' => 255,
        'enable_tokens'   => 0,
        'attributes'      => array(
          'target' => '_blank',
        ),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'link_label',
          'weight' => 12,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 17,
      ),
    ),
    'link_to_publisher' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_publisher',
      'label'       => $t('Link to publisher'),
      'description' => $t('Link to the publisher\'s website. The "title" is the text which will appear on the button.'),
      'required'    => FALSE,
      'settings'    => array(
        'absolute_url'    => 1,
        'validate_url'    => 1,
        'url'             => 0,
        'title'           => 'required',
        'title_maxlength' => 255,
        'enable_tokens'   => 0,
        'attributes'      => array(
          'target' => '_blank',
        ),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'link_label',
          'weight' => 13,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 18,
      ),
    ),
    'link_to_buy' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_link_to_buy',
      'label'       => $t('Link to buy'),
      'description' => $t('Link to a supplier which sells the full book. The "title" is the text which will appear on the button.'),
      'required'    => FALSE,
      'settings'    => array(
        'absolute_url'    => 1,
        'validate_url'    => 1,
        'url'             => 0,
        'title'           => 'required',
        'title_maxlength' => 255,
        'enable_tokens'   => 0,
        'attributes'      => array(
          'target' => '_blank',
        ),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'link_label',
          'weight' => 14,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 19,
      ),
    ),
    'weight' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_weight",
      'label'       => $t('Weight'),
      'description' => $t('In ordered lists, where should this node rank? Heavier items sink to the bottom, and lighter items float to the top.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'type' => 'hidden',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 22,
      ),
    ),
    'featured_image' => array(
      'field_name'  => RSC_LIBRARY_PREFIX."_featured_image",
      'label'       => $t('Featured image'),
      'description' => $t('An image to display along with this node to attract attention.'),
      'required'    => FALSE,
      'settings'    => array(
        'file_directory'  => 'featured_images',
        'file_extensions' => 'png gif jpg jpeg',
        'alt_field'       => 1,
        'title_field'     => 0,
      ),
      'display' => array(
        'default' => array(
          'type' => 'hidden',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'label' => 'hidden',
          'type' => 'image',
          'weight' => 0,
          'settings' => array(
//            'image_style' => 'sidebar_thumbnail',
// must be set manually, unless we depend on manualcrop module and define all of its settings in the rsc_library module (probably hard to do)
            'image_link' => 'content',
          ),
        ),
      ),
      'widget' => array(
        'weight' => 23,
        'type' => 'image_image',
        'settings' => array(
          // must be set manually, unless we depend on manualcrop module and define all of its settings in the rsc_library module (probably hard to do)
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
          // must be set manually, unless we depend on filefield_sources module and define all of its settings in the rsc_library module (probably hard to do)
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
    'full_text' => array(
      'field_name'  => RSC_LIBRARY_PREFIX.'_full_text',
      'label'       => $t('Full text'),
      'description' => $t('The entire article, formatted using basic HTML elements. Do not use custom fonts, sizes or colors here, since those should be governed by the website\'s theme.'),
      'required'    => FALSE,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'text_default',
          'weight' => 16,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'weight' => 24,
      ),
    ),
  );

  // define the field instances for each content type, because not all content
  // types use all the fields
  $field_instances_by_type = array(
    'rsc_library_article' => array(
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['full_text'],
      $field_instances_on_nodes['attachment'] + array(
        'settings' => array(
          'file_extensions' => 'pdf mp3 ogg mp4 avi',
          'file_directory'  => 'articles',
        ),
      ),
      $field_instances_on_nodes['pages'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['format'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['featured_image'],
    ),
    'rsc_library_link' => array(
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['link'],
      $field_instances_on_nodes['pages'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['format'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['featured_image'],
    ),
    'rsc_library_request' => array(
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['book_title'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['page_range'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['format'],
      $field_instances_on_nodes['featured_image'],
    ),
    'rsc_library_book_preview' => array(
      $field_instances_on_nodes['summary'],
      $field_instances_on_nodes['code'],
      $field_instances_on_nodes['book_title'],
      $field_instances_on_nodes['date'],
      $field_instances_on_nodes['page_range'],
      $field_instances_on_nodes['copyright_notice'],
      $field_instances_on_nodes['attachment'] + array(
        'settings' => array(
          'file_extensions' => 'pdf mp3',
          'file_directory'  => 'book_previews',
        ),
      ),
      $field_instances_on_nodes['link_to_chapter'],
      $field_instances_on_nodes['link_to_publisher'],
      $field_instances_on_nodes['link_to_buy'],
      $field_instances_on_nodes['weight'],
      $field_instances_on_nodes['featured_image'],
    ),
  );

  // add the field instances to the content types
  foreach ($field_instances_by_type as $type => $field_instances) {
    rsc_library_field__setup_field_instances_on_bundle('node', $type, $field_instances);
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
      'description' => $t('Who wrote this?'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 2,
        ),
        'teaser' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 0,
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 2,
        ),
      ),
      'widget'      => array(
        'type' => 'taxonomy_autocomplete',
        'weight' => 4,
      ),
    ),
    'editor' => array(
      'label'       => $t('Editor'),
      'description' => $t('Who edited this?'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 3,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type' => 'taxonomy_autocomplete',
        'weight' => 5,
      ),
    ),
    'publisher' => array(
      'label'       => $t('Publisher'),
      'description' => $t('The publisher of this book.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 5,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type' => 'taxonomy_autocomplete',
        'weight' => 7,
      ),
    ),
    'source' => array(
      'label'       => $t('Source'),
      'description' => $t('Where this content comes from, e.g. the magazine in which an article was published.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 6,
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type' => 'taxonomy_autocomplete',
        'weight' => 9,
      ),
    ),
    'difficulty' => array(
      'label'       => $t('Difficulty'),
      'description' => $t('How difficult or technical is this content?'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 1,
        ),
        'teaser' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 1,
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'label' => 'hidden',
          'type' => 'taxonomy_term_reference_link',
          'weight' => 1,
        ),
      ),
      'widget'      => array(
        'type' => 'options_buttons',
        'weight' => 14,
      ),
    ),
    'tag' => array(
      'label'       => $t('Tags'),
      'description' => $t('Keywords relating to this content.'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'type' => 'hidden',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
          'type' => 'hidden',
        ),
      ),
      'widget'      => array(
        'type' => 'taxonomy_autocomplete',
        'weight' => 20,
      ),
    ),
    'category' => array(
      'label'       => $t('Category (in "@lib")', array('@lib'=>$library_title)),
      'description' => $t('Where in the library should this be located?'),
      'required'    => FALSE,
      'display' => array(
        'default' => array(
          'type' => 'hidden',
        ),
        'teaser' => array(
          'type' => 'hidden',
        ),
        RSC_LIBRARY_PREFIX . '_block' => array(
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
        'weight' => 21,
      ),
    ),
  );

  // first check that each field has an assigned vocabulary
  foreach($taxonomy_reference_field_instances as $field_short_name => $field_info) {

    $error = FALSE;

    if (empty($library_settings['vocabs'][$field_short_name]) || !($library_settings['vocabs'][$field_short_name] > 0)) {
      drupal_set_message($t('Could not create @field field for library @lid, because a vocabulary has not been selected yet.', array(
        '@field' => $field_short_name,
        '@lid' => $lid,
      )), 'warning', FALSE);
      $error = TRUE;
    }

    if ($error) return;

  }

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
      $taxonomy_reference_field_instances['publisher'],
      $taxonomy_reference_field_instances['tag'],
      $taxonomy_reference_field_instances['category'],
      $taxonomy_reference_field_instances['difficulty'],
    ),
    'rsc_library_book_preview' => array(
      $taxonomy_reference_field_instances['author'],
      $taxonomy_reference_field_instances['editor'],
      $taxonomy_reference_field_instances['publisher'],
      $taxonomy_reference_field_instances['tag'],
      $taxonomy_reference_field_instances['category'],
      $taxonomy_reference_field_instances['difficulty'],
    ),
  );

  // add the field instances to the content types
  foreach ($field_instances_by_type as $type => $field_instances) {
    rsc_library_field__setup_field_instances_on_bundle('node', $type, $field_instances);
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
      'required'    => FALSE,
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
      'required'    => FALSE,
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
  rsc_library_field__setup_field_instances_on_bundle('taxonomy_term', $category_vocabulary->machine_name, $field_instances_on_taxonomy_terms);

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
function rsc_library_field__setup_field_instances_on_bundle($entity_type, $bundle, $new_field_instances) {

  // Get the proper translation function, since this function might run during install, or any other time
  $t = get_t();

  // Get a list of existing fields and field instances
  $existing_fields = array_keys(field_info_fields());
  $existing_field_instances = array_keys(field_info_instances($entity_type, $bundle));

  // Create each field instance if it does not exist yet
  foreach ($new_field_instances as $new_field_instance) {
    $field_name = $new_field_instance['field_name'];

    // check that the field exists
    if (!in_array($field_name, $existing_fields)) {
      drupal_set_message($t("Cannot create field instance, because field @field does not exist.", array('@field'=>$field_name)), 'error', FALSE);
      continue;
    }

    $new_field_instance['entity_type'] = $entity_type;
    $new_field_instance['bundle'] = $bundle;

    if (in_array($field_name, $existing_field_instances)) {
      // update an existing field instance
      $existing_field_instance = field_info_instance($entity_type, $field_name, $bundle);
      $updated_field_instance = array_replace_recursive($existing_field_instance, $new_field_instance);
      field_update_instance($updated_field_instance);
    } else {
      // create a new field instance
      field_create_instance($new_field_instance);
      $existing_field_instances[] = $field_name;
    }

  }

}

