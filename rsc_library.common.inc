<?php


/**
 * (re)set content type settings
 */
function rsc_library_content_type_settings($content_types = array()) {
  
  if (!is_array($content_types) || empty($content_types)) {
    $content_types = array(
      'rsc_library_article',
      'rsc_library_audio',
    );
  }
  foreach ($content_types as $type) {
    // settings from the content type manage page
    variable_set("node_preview_{$type}", 1);                  // Optional preview
    variable_set("node_options_{$type}", array());           // default unpublished
    variable_set("node_submitted_{$type}", 0);                // Unset 'Display author and date information'.
    variable_set("comment_{$type}", COMMENT_NODE_OPEN);       // open comments
    variable_set("menu_options_{$type}", array());           // no available menus
  }
  
  // pathauto settings
  variable_set("pathauto_node_rsc_library_article_pattern", 'article/[node:title]');
  variable_set("pathauto_node_rsc_library_audio_pattern", 'audio/[node:title]');
  
}


/**
 * (re)create fields
 * FIXME: this uses outdated $settings variable when function is called in submit handler.
 * Workaround: submit form once for setting vocabs, second time for creating fields
 */
function rsc_library_field_config() {

  $fieldnames = array_keys(field_info_fields());
  $prefix = 'rscl'; // using this stupid prefix because of the 32 char limit on field names. Stands for RSC Library

  // body text
  $fieldname = "{$prefix}_body";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text_long',
      'cardinality' => 1,
    ));
  }

  // attachment
  $fieldname = "{$prefix}_attachment";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'file',
      'cardinality' => 1,
      'settings'    => array(
        'uri_scheme' => 'public',
      ),
    ));
  }

  // pages
  $fieldname = "{$prefix}_pages";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'number_integer',
      'cardinality' => 1,
    ));
  }

  // weight
  $fieldname = "{$prefix}_weight";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'number_integer',
      'cardinality' => 1,
    ));
  }

  // format
  $fieldname = "{$prefix}_format";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'list_text',
      'cardinality' => 1,
    ));
  }

  // source
  $fieldname = "{$prefix}_source";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text',
      'cardinality' => 1,
    ));
  }

  // editor
  $fieldname = "{$prefix}_editor";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text',
      'cardinality' => 1,
    ));
  }

  // code
  $fieldname = "{$prefix}_code";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text',
      'cardinality' => 1,
    ));
  }

  // publisher
  $fieldname = "{$prefix}_publisher";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text',
      'cardinality' => 1,
    ));
  }

  // link
  $fieldname = "{$prefix}_link";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text',
      'cardinality' => 1,
    ));
  }

  // date
  $fieldname = "{$prefix}_date";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'text',
      'cardinality' => 1,
    ));
  }
  
  // featured
  $fieldname = "{$prefix}_featured";
  if (!in_array($fieldname,$fieldnames)) {
    field_create_field(array(
      'field_name'  => $fieldname,
      'type'        => 'list_boolean',
      'cardinality' => 1,
      'settings'    => array(
        'allowed_values' => array(
          0 => '',
          1 => '',
        ),
      ),
    ));
  }

  // taxonomy reference fields
  $settings = variable_get('rsc_library',array());
  $cardinality = array(
    'author'     => 1,
    'tag'        => FIELD_CARDINALITY_UNLIMITED,
    'category'   => 1,
    'difficulty' => 1,
  );

  if (!empty($settings)) {
    foreach($settings as $lid => $lib_settings) {          // for each library
      foreach($lib_settings['vocabs'] as $field => $vid) { // for each taxonomy reference field
        if ($vid > 0) {                                     // if a vocabulary has been configured
          $vocab = taxonomy_vocabulary_load($vid);
          $fieldname = "{$prefix}_{$field}_{$vocab->machine_name}"; // determine what this field should be called
          if (strlen($field_name) <= 32) { 
            if (!in_array($fieldname, $fieldnames)) {                 // create it if it doesn't exist yet
              field_create_field(array(
                'field_name'  => $fieldname,
                'type'        => 'taxonomy_term_reference',
                'cardinality' => $cardinality[$field],
                'settings'    => array(
                  'allowed_values' => array(
                    array(
                      'vocabulary' => $vocab->machine_name,
                      'parent'     => '0',
                    ),
                  ),
                ),
              ));
            }
          } else {
            drupal_set_message(t('Cannot create field @fieldname (more than 32 characters).', array('@fieldname' => $fieldname)),'error');
          }
        }
      }
    }
  }

}


/**
 * (re)configure field instances
 */
function rsc_library_field_instance_config() {
  $t = get_t(); // this function might run during install, or any other time
  $prefix = "rscl";

  /* Field instances for articles */

  $instances = array_keys(field_info_instances('node','rsc_library_article'));
  $fields = array_keys(field_info_fields());

  $fieldname = "{$prefix}_body";
  if (!in_array($fieldname, $instances) && in_array($fieldname, $fields)) {
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
  }

  $fieldname = "{$prefix}_attachment";
  if (!in_array($fieldname, $instances) && in_array($fieldname, $fields)) {
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
  }

  // The next few instances are almost identical
  $fieldlabels = array(
    'pages'     => $t('Pages'),
    'weight'    => $t('Weight'),
    'format'    => $t('Format'),
    'source'    => $t('Source'),
    'editor'    => $t('Editor'),
    'code'      => $t('Code'),
    'publisher' => $t('Publisher'),
    'link'      => $t('Link'),
    'date'      => $t('Date'),
  );

  foreach ($fieldlabels as $fname => $label) {
    $fieldname = "{$prefix}_{$fname}";
    if (!in_array($fieldname, $instances) && in_array($fieldname, $fields)) {
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
    }
  }

  // taxonomy reference field instances
  $settings = variable_get('rsc_library',array());
  $label = array(
    'author'     => $t('Author'),
    'tag'        => $t('Tags'),
    'category'   => $t('Category'),
    'difficulty' => $t('Difficulty'),
  );
  $widget = array(
    'category'   => array(
      'type'     => 'taxonomy_shs',
      'settings' => array(
        'shs' => array(
          'node_count' => FALSE,
          'create_new_terms' => FALSE,
          'create_new_levels' => FALSE,
          'force_deepest' => FALSE,
        ),
      ),
    ),
    'author'     => array('type' => 'options_select'),
    'tag'        => array('type' => 'options_select'),
    'difficulty' => array('type' => 'options_select'),
  );

  if (!empty($settings)) {
    foreach($settings as $lid => $lib_settings) {          // for each library
      foreach($lib_settings['vocabs'] as $field => $vid) { // for each taxonomy reference field
        if ($vid > 0) {                                     // if a vocabulary has been configured
          $vocab = taxonomy_vocabulary_load($vid);
          $fieldname = "{$prefix}_{$field}_{$vocab->machine_name}";                 // determine what this field is called
          if (!in_array($fieldname, $instances) && in_array($fieldname, $fields)) { // create it if it doesn't exist yet
            field_create_instance(array(
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
          }
        }
      }
    }
  }

  /* Field instances for audio */

  // TODO!
  
  /* Field instances for 'category' taxonomy terms */
  
  if (!empty($settings)) {
    foreach($settings as $lid => $lib_settings) { // for each library
      
      // featured
      $fieldname = "{$prefix}_featured";
      $vocab = taxonomy_vocabulary_load($lib_settings['vocabs']['category']);
      if (!in_array($fieldname, $instances) && in_array($fieldname, $fields)) {
        field_create_instance(array(
          'field_name'  => $fieldname,
          'entity_type' => 'taxonomy_term',
          'bundle'      => $vocab->machine_name,
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
      }
      
    }
  }

}


/**
 * Delete unused fields that start with rsc_library_
 * Unused means: does not have any instances
 */
function rsc_library_field_cleanup() {
  $fields = array_keys(field_info_fields());
  $used_fields = array_keys(field_info_field_map());
  $unused_fields = array_diff($fields,$used_fields);

  foreach ($unused_fields as $field) {
    field_delete_field($field);
  }
}

