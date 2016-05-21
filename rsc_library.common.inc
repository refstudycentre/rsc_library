<?php

/*
 * The rsc_library module defines these content types in addition to the rsc_library type.
 * See rsc_library_node_info().
 */
define('RSC_LIBRARY_CONTENT_TYPES_INFO', array(
  'rsc_library_article' => array(
    'name'        => t('Library content'),
    'base'        => 'rsc_library_article',
    'description' => t("Own content, whether text, audio, video, or combination of those."),
    'locked'      => TRUE,  // admin cannot change machine name
  ),
  'rsc_library_link' => array(
    'name'        => t('External content'),
    'base'        => 'rsc_library_link',
    'description' => t("Link to content hosted elsewhere."),
    'locked'      => TRUE,  // admin cannot change machine name
  ),
  'rsc_library_request' => array(
    'name'        => t('Request-able content'),  // TODO: get a better name for this? "Requests"?
    'base'        => 'rsc_library_request',
    'description' => t("Content that may be requested via email"),
    'locked'      => TRUE,  // admin cannot change machine name
  ),
  'rsc_library_book_preview' => array(
    'name'        => t('Book preview'),
    'base'        => 'rsc_library_book',
    'description' => t("A book, usually for sale, of which we host the first chapter"),
    'locked'      => TRUE,  // admin cannot change machine name
  ),
));
define('RSC_LIBRARY_CONTENT_TYPES', array_keys(RSC_LIBRARY_CONTENT_TYPES_INFO));

// Field names are limited to 32 characters, so we use a short prefix:
define('RSC_LIBRARY_PREFIX', 'rscl');

// Fields defined by this module, that do not change based on user settings:
define('RSC_LIBRARY_FIELDS_STATIC', array(
  RSC_LIBRARY_PREFIX.'_body' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_body',
    'type'        => 'text_long',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_attachment' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_attachment',
    'type'        => 'file',
    'cardinality' => 1,
    'settings'    => array(
      'uri_scheme' => 'public',
    ),
  ),
  RSC_LIBRARY_PREFIX.'_pages' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_pages',
    'type'        => 'number_integer',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_weight' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_weight',
    'type'        => 'number_integer',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_format' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_format',
    'type'        => 'list_text',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_editor' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_editor',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_code' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_code',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_publisher' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_publisher',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_link' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_link',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_date' => array(
    'field_name'  => RSC_LIBRARY_PREFIX.'_date',
    'type'        => 'text',
    'cardinality' => 1,
  ),
  RSC_LIBRARY_PREFIX.'_featured' => array(
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
  RSC_LIBRARY_PREFIX.'_sort_enabled' => array(
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
 * (re)set content type settings
 */
function rsc_library_content_type_settings($content_types = RSC_LIBRARY_CONTENT_TYPES) {

  // for each "rsc_library_*" content type
  foreach ($content_types as $type) {
    // settings from the content type manage page
    variable_set("node_preview_{$type}", 1);                 // Optional preview
    variable_set("node_options_{$type}", array());           // default unpublished
    variable_set("node_submitted_{$type}", 0);               // Unset 'Display author and date information'.
    variable_set("comment_{$type}", COMMENT_NODE_OPEN);      // open comments
    variable_set("menu_options_{$type}", array());           // no available menus
  }

  // for the "rsc_library" content type
  variable_set("node_preview_rsc_library", 1);               // Optional preview
  variable_set("node_options_rsc_library", array());         // default unpublished
  variable_set("node_submitted_rsc_library", 0);             // Unset 'Display author and date information'.
  variable_set("comment_rsc_library", COMMENT_NODE_CLOSED);  // close comments
  variable_set("menu_options_rsc_library", array());         // no available menus
  
  // pathauto settings
  variable_set("pathauto_node_rsc_library_article_pattern", 'article/[node:title]');
  variable_set("pathauto_node_rsc_library_link_pattern", 'link/[node:title]');
  variable_set("pathauto_node_rsc_library_request_pattern", 'request/[node:title]');
  variable_set("pathauto_node_rsc_library_book_pattern", 'book-chapter/[node:title]');
  
}


/**
 * (re)create fields
 * FIXME: this uses outdated $settings variable if function is called in a library node edit form submit handler.
 * Workaround: submit form once for setting vocabs, second time for creating fields
 */
function rsc_library_field_config($lid = NULL) {

  // Get a list of library IDs for which to configure the fields
  $lids = array();
  if ($lid == NULL) {
    $lids = rsc_library_get_lids();
  } else if (is_numeric($lid)) {
    $lids = array($lid);
  }
  
  // body text with summary for library front page
  node_add_body_field(node_type_get_type('rsc_library'));

  // get a list of existing fields, so that we don't add duplicates:
  $field_names = array_keys(field_info_fields());

  // add fields that do not change based on user settings:
  foreach (RSC_LIBRARY_FIELDS_STATIC as $field_name => $field_info) {
    if (!in_array($field_name, $field_names)) {
      field_create_field($field_info);
    }
  }

  /* Create taxonomy reference fields: */

  // for each library
  foreach($lids as $lid) {

    // get the vocabulary settings for this library
    $settings = variable_get("rsc_library_{$lid}", NULL);
    if (empty($settings)) {
      drupal_set_message(t('Could not create fields for library @lid, because its vocabulary settings have not been configured yet.', array('@lid' => $lid)), 'warning');
    } else {
      rsc_library_create_taxonomy_fields($settings);
    }

  }

}


/**
 * Create "taxonomy term reference" fields on a specific library
 */
function rsc_library_create_taxonomy_fields($library_settings) {

  // cardinality settings for the taxonomy fields added below
  static $cardinality = array(
    'author'     => FIELD_CARDINALITY_UNLIMITED,
    'tag'        => FIELD_CARDINALITY_UNLIMITED,
    'category'   => FIELD_CARDINALITY_UNLIMITED,
    'difficulty' => 1,
    'source'     => 1,
  );

  // get a list of existing fields, so that we don't add duplicates:
  $field_names = array_keys(field_info_fields());

  // for each taxonomy reference field
  foreach ($library_settings['vocabs'] as $field => $vid) {

    // if a vocabulary has been configured
    if ($vid > 0) {

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
}


/**
 * (re)configure field instances
 */
function rsc_library_field_instance_config($lid = NULL) {
  $t = get_t(); // this function might run during install, or any other time
  $fields = array_keys(field_info_fields());
  
  $lids = array();
  if ($lid == NULL) {
    $lids = rsc_library_get_lids();
  } else if (is_numeric($lid)) {
    $lids = array($lid);
  }
  
  /* Field instances for audio */
  
  $instances_audio = array_keys(field_info_instances('node','rsc_library_audio'));
  
  $fieldname = RSC_LIBRARY_PREFIX.'_body';
  if (!in_array($fieldname, $instances_audio) && in_array($fieldname, $fields)) {
    field_create_instance(array(
      'field_name'  => $fieldname,
      'entity_type' => 'node',
      'bundle'      => 'rsc_library_audio',
      'label'       => $t('Summary'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'label' => 'hidden',
        ),
      ),
    ));
    $instances_audio[] = $fieldname;
  }
  
  $fieldname = RSC_LIBRARY_PREFIX.'_attachment';
  if (!in_array($fieldname, $instances_audio) && in_array($fieldname, $fields)) {
    field_create_instance(array(
      'field_name'  => $fieldname,
      'entity_type' => 'node',
      'bundle'      => 'rsc_library_audio',
      'label'       => $t('Attachment'),
      'required'    => false,
      'settings'    => array(
        'file_extensions' => 'mp3',
        'file_directory'  => 'audio',
      ),
      'display'     => array(
        'default' => array(
          'label' => 'hidden',
        ),
      ),
    ));
    $instances_audio[] = $fieldname;
  }
  
  $fieldname = RSC_LIBRARY_PREFIX.'_link';
  if (!in_array($fieldname, $instances_audio) && in_array($fieldname, $fields)) {
    field_create_instance(array(
      'field_name'  => $fieldname,
      'entity_type' => 'node',
      'bundle'      => 'rsc_library_audio',
      'label'       => $t('Link'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ));
    $instances_audio[] = $fieldname;
  }
  
  // The next few instances are almost identical
  $fieldlabels = array(
    'weight'    => $t('Weight'),
    'code'      => $t('Code'),
  );
  
  foreach ($fieldlabels as $fname => $label) {
    $fieldname = RSC_LIBRARY_PREFIX."_{$fname}";
    if (!in_array($fieldname, $instances_audio) && in_array($fieldname, $fields)) {
      field_create_instance(array(
        'field_name'  => $fieldname,
        'entity_type' => 'node',
        'bundle'      => 'rsc_library_audio',
        'label'       => $label,
        'required'    => false,
        'display'     => array(
          'default' => array(
            'type' => 'hidden',
          ),
        ),
      ));
      $instances_audio[] = $fieldname;
    }
  }

  /* Field instances for articles */
  
  $instances_article = array_keys(field_info_instances('node','rsc_library_article'));

  $fieldname = RSC_LIBRARY_PREFIX.'_body';
  if (!in_array($fieldname, $instances_article) && in_array($fieldname, $fields)) {
    field_create_instance(array(
      'field_name'  => $fieldname,
      'entity_type' => 'node',
      'bundle'      => 'rsc_library_article',
      'label'       => $t('Summary'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'label' => 'hidden',
        ),
      ),
    ));
    $instances_article[] = $fieldname;
  }

  $fieldname = RSC_LIBRARY_PREFIX.'_attachment';
  if (!in_array($fieldname, $instances_article) && in_array($fieldname, $fields)) {
    field_create_instance(array(
      'field_name'  => $fieldname,
      'entity_type' => 'node',
      'bundle'      => 'rsc_library_article',
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
    ));
    $instances_article[] = $fieldname;
  }
  
  $fieldname = RSC_LIBRARY_PREFIX.'_link';
  if (!in_array($fieldname, $instances_article) && in_array($fieldname, $fields)) {
    field_create_instance(array(
      'field_name'  => $fieldname,
      'entity_type' => 'node',
      'bundle'      => 'rsc_library_article',
      'label'       => $t('Link'),
      'required'    => false,
      'settings'    => array('text_processing' => 1), // enable rich text formats
      'display'     => array(
        'default' => array(
          'type' => 'hidden',
        ),
      ),
    ));
    $instances_article[] = $fieldname;
  }

  // The next few instances are almost identical
  $fieldlabels = array(
    'pages'     => $t('Pages'),
    'weight'    => $t('Weight'),
    'format'    => $t('Format'),
    'editor'    => $t('Editor'),
    'code'      => $t('Code'),
    'publisher' => $t('Publisher'),
    'date'      => $t('Date'),
  );

  foreach ($fieldlabels as $fname => $label) {
    $fieldname = RSC_LIBRARY_PREFIX."_{$fname}";
    if (!in_array($fieldname, $instances_article) && in_array($fieldname, $fields)) {
      field_create_instance(array(
        'field_name'  => $fieldname,
        'entity_type' => 'node',
        'bundle'      => 'rsc_library_article',
        'label'       => $label,
        'required'    => false,
        'display'     => array(
          'default' => array(
            'type' => 'hidden',
          ),
        ),
      ));
      $instances_article[] = $fieldname;
    }
  }
  
  // values that are being reused in the foreach below
  $widget = array(
    'category'   => array(
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
    'author'     => array('type' => 'taxonomy_autocomplete'),
    'source'     => array('type' => 'taxonomy_autocomplete'),
    'tag'        => array('type' => 'taxonomy_autocomplete'),
    'difficulty' => array('type' => 'options_buttons'),
  );

  foreach ($lids as $lid) { // for each library
    $settings = variable_get("rsc_library_{$lid}",NULL);
    $cat_vocab = taxonomy_vocabulary_load($settings['vocabs']['category']);
    $instances_cat = array_keys(field_info_instances('taxonomy_term',$cat_vocab->machine_name));
    if (!empty($settings)) {
      
      $node_front = node_load($lid);
      $label = array(
        'author'     => $t('Author'),
        'source'     => $t('Source'),
        'tag'        => $t('Tags'),
        'category'   => $t('Category (in "@lib")',array('@lib'=>$node_front->title)),
        'difficulty' => $t('Difficulty'),
      );
      
      // taxonomy reference field instances for articles
      foreach($settings['vocabs'] as $field => $vid) {             // for each taxonomy reference field
        if ($vid > 0) {                                             // if a vocabulary has been configured
          $vocab = taxonomy_vocabulary_load($vid);
          $fieldname = RSC_LIBRARY_PREFIX."_{$field}_{$vocab->machine_name}"; // determine what this field is called
          if (in_array($fieldname, $fields)) {                      // if the field exists

            // for articles
            if (!in_array($fieldname, $instances_article)) { // if the instance does not exist yet
              field_create_instance(array(                   // create the instance
                'field_name'  => $fieldname,
                'entity_type' => 'node',
                'bundle'      => 'rsc_library_article',
                'label'       => $label[$field],
                'required'    => false,
                'display'     => array(
                  'default' => array(
                    'type' => 'hidden',
                  ),
                ),
                'widget'      => $widget[$field],
              ));
              $instances_article[] = $fieldname;
            }
            
            // for audio
            if (!in_array($fieldname, $instances_audio)) { // if the instance does not exist yet
              field_create_instance(array(                 // create the instance
                'field_name'  => $fieldname,
                'entity_type' => 'node',
                'bundle'      => 'rsc_library_audio',
                'label'       => $label[$field],
                'required'    => false,
                'display'     => array(
                  'default' => array(
                    'type' => 'hidden',
                  ),
                ),
                'widget'      => $widget[$field],
              ));
              $instances_audio[] = $fieldname;
            }
            
          } else {
            drupal_set_message(t('Cannot create instance of non-existing field <strong>@fieldname</strong> on taxonomy bundle <strong>@bundle</strong> for library <strong>@lid</strong>.',array('@fieldname'=>$fieldname,'@bundle'=>$vocab->machine_name,'@lid'=>$lid)),'error');
          }
        }
      }
    
      // featured field instance for 'category' taxonomy terms
      $fieldname = RSC_LIBRARY_PREFIX."_featured";
      if (!in_array($fieldname, $instances_cat) && in_array($fieldname, $fields)) { // create it if it doesn't exist yet
        field_create_instance(array(
          'field_name'  => $fieldname,
          'entity_type' => 'taxonomy_term',
          'bundle'      => $cat_vocab->machine_name,
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
        ));
        $instances_cat[] = $fieldname; // add instance to list of names to prevent duplicate creation attempts
      }
    
      // sort_enabled field instance for 'category' taxonomy terms
      $fieldname = RSC_LIBRARY_PREFIX."_sort_enabled";
      if (!in_array($fieldname, $instances_cat) && in_array($fieldname, $fields)) { // create it if it doesn't exist yet
        field_create_instance(array(
          'field_name'  => $fieldname,
          'entity_type' => 'taxonomy_term',
          'bundle'      => $cat_vocab->machine_name,
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
        ));
        $instances_cat[] = $fieldname; // add instance to list of names to prevent duplicate creation attempts
      }
      
    }
  }

}


/**
 * Delete unused fields that start with RSC_LIBRARY_PREFIX_
 * Unused means: does not have any instances
 */
function rsc_library_field_cleanup() {
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
 * Get all library ids
 */
function rsc_library_get_lids() {
  $lids = array();
  
  $q = new EntityFieldQuery();
  $q->entityCondition('entity_type','node')
    ->entityCondition('bundle','rsc_library')
    ->addMetaData('account', user_load(1));
  $r = $q->execute();
  if (!empty($r['node'])) {
    $lids = array_keys($r['node']);
  }
  
  return $lids;
}

