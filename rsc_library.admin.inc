<?php


/**
 * Form builder for the module settings page
 */
function rsc_library_settings() {
  $form = array();
  
  /* collect info from site*/
  $vocabs = taxonomy_get_vocabularies();
  $vocab_options = array('-1' => t('None'));
  foreach($vocabs as $vid => $vocab) {
    $vocab_options[$vid] = $vocab->name;
  }
  
  /* collect info from this module */
  $default_settings = array();
  //$settings = variable_get('rsc_library',$default_settings);
  
  /* build the form */
  $form['#tree'] = TRUE; // save the whole thing as an array
  
  $form['rsc_library']['actions'] = array(
    '#type'              => 'radios',
    '#title'             => 'Actions:',
    '#default_value'     => 'Nothing',
    '#options'           => array(
      0                        => 'Nothing',
      'content_type_reset'     => 'Reset the content type settings.',
      'fields_create'          => '(re)Create missing fields and instances',
      'fields_cleanup'         => 'Delete (clean up) unused fields',
    ),
    '#description'       => t('Choose an optional action to perform when submitting.'),
    'content_type_reset' => array('#description' => t("Default settings include: unpublished by default, comments open, etc.")),
    'fields_create'      => array('#description' => t("Old fields and field instances will not be deleted automatically. If you are experiencing trouble with a field or field instance, delete it, and the choose this option. YOU WILL LOSE ALL VALUES CONTAINED IN EVERY INSTANCE OF A FIELD IF YOU DELETE IT.")),
    'fields_cleanup'     => array('#description' => t("This will delete all fields created by rsc_library which do not have any instances (i.e. are not linked to a content type any more). YOU WILL LOSE ALL DATA IN THESE FIELDS.")),
  );
  
  $links = array();
  $links["admin/structure/types/manage/rsc-library"] = t('Edit "library" content type');
  $links["admin/structure/types/manage/rsc-library-article"] = t('Edit "article" content type');
  $links["admin/structure/types/manage/rsc-library-audio"] = t('Edit "audio" content type');
  
  $link_items = array();
  foreach($links as $path => $text) {
    $link_items[] = l($text,$path);
  }
  $link_list = array(
    '#title'  => t('Related configuration pages:'),
    '#theme'  => 'item_list',
    '#items'  => $link_items,
  );
  
  $form['rsc_library']['links'] = array(
    '#type'   => 'markup',
    '#markup' => drupal_render($link_list),
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
  );
  
  $form['#submit'][] = 'rsc_library_settings_submit';

  return system_settings_form($form);
}


/**
 * Submit handler for the module settings page
 */
function rsc_library_settings_submit($form, &$form_state) {
  
  if (!empty($form_state['values']['rsc_library'])) {
    $settings = $form_state['values']['rsc_library'];
      
    switch($settings['actions']) {
      case 'content_type_reset':
        
        module_load_include('inc','rsc_library','rsc_library.common');
        rsc_library_content_type_settings();
        drupal_set_message("Settings for content types rsc_library_* have been reset.");
        
        break;
      case 'fields_cleanup':
        
        module_load_include('inc','rsc_library','rsc_library.common');
        rsc_library_field_cleanup();
        drupal_set_message("Unused fields from rsc_library have been deleted.");
        
        break;
      case 'fields_create':
        
        module_load_include('inc','rsc_library','rsc_library.common');
        rsc_library_field_config();
        rsc_library_field_instance_config();
        drupal_set_message("Fields and instances for rsc_library have been created.");
        
        break;
    }
      
  }
}


/**
 * Form builder for the node settings page
 */
function rsc_library_node_settings($node = NULL) {
  $form = array();
  
  if (!empty($node->nid) && is_numeric($node->nid)) { // if this is an existing library
  
    $links = array();
    
    /* collect info from site */
    $vocabs = taxonomy_get_vocabularies();
    $vocab_options = array('-1' => t('None'));
    foreach($vocabs as $vid => $vocab) {
      $vocab_options[$vid] = $vocab->name;
    }
  
    /* collect info from this library */
    $default_settings = array(
      'vocabs' => array(
        'category'   => '-1',
        'tag'        => '-1',
        'author'     => '-1',
        'source'     => '-1',
        'difficulty' => '-1',
      ),
    );
      
    $settings = variable_get("rsc_library_{$node->nid}",$default_settings);
    
    $vocab_cat = taxonomy_vocabulary_load($settings['vocabs']['category']);
    $menu_name = "rsc_library_{$node->nid}";
    $menus = menu_get_menus();
  
    /* build link list */
    if (!empty($vocab_cat)) {
      $links["admin/structure/taxonomy/{$vocab_cat->machine_name}/edit"] = t('Edit category vocabulary');
      $links["admin/structure/taxonomy_manager/voc/{$vocab_cat->machine_name}"] = t('Manage category terms');
    }
    $links["admin/structure/taxonomy/add"] = t('Add a new vocabulary');
    if (isset($menu_name) && isset($menus[$menu_name])) {
      $links["admin/structure/menu/manage/{$menu_name}/edit"] = t('Edit category menu');
    }
    $link_items = array();
    foreach($links as $path => $text) {
      $link_items[] = l($text,$path);
    }
    $link_list = array(
      '#title'  => t('Related configuration pages:'),
      '#theme'  => 'item_list',
      '#items'  => $link_items,
    );
  
    /* build the form */
    $form['#tree'] = TRUE; // save the whole thing as an array
    
    $form['links'] = array(
      '#type'   => 'markup',
      '#markup' => drupal_render($link_list),
      '#prefix' => '<div class="form-item">',
      '#suffix' => '</div>',
    );
  
    $form['vocabs'] = array(
      '#type'        => 'fieldset',
      '#title'       => 'Taxonomy vocabulary to use for...',
    );
  
    $form['vocabs']['category'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('categories'),
      '#description'   => t('A menu will be created and populated with the terms of this vocabulary. <br/> A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => $settings['vocabs']['category'],
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['author'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('authors'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => $settings['vocabs']['author'],
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['source'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('sources'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => $settings['vocabs']['source'],
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['tag'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('tags'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => $settings['vocabs']['tag'],
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['difficulty'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('"difficulty" levels'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => $settings['vocabs']['difficulty'],
      '#options'       => $vocab_options,
    );
  
    $form['actions'] = array(
      '#type'              => 'radios',
      '#title'             => 'Actions:',
      '#default_value'     => 'Nothing',
      '#options'           => array(
        0                        => 'Nothing',
        'menu_rebuild'           => '(re)Create and populate the category menu.',
      ),
      '#description'           => t('Choose an optional action to perform when submitting.'),
      'menu_rebuild'           => array('#description' => t("This might take a while!")),
    );
  
    $form['#submit'] = 'rsc_library_node_settings_submit';
    
  } else {
    $form['#markup'] = "<p>Save the library and edit it again to see more options ...</p><p><small>This is because the library node needs and 'nid' before we can save the relevant variables.</small></p>";
  }
  
  return $form;

}


/**
 * Submit handler for the node settings page
 */
function rsc_library_node_settings_submit($form, &$form_state) {
  
  $title = $form_state['values']['title'];
  $values = $form_state['values']['rsc_library_node_settings'];
  $nid = $form_state['values']['nid'];
  if (is_numeric($nid) && is_array($values) && isset($values['vocabs'])) {
    variable_set("rsc_library_{$nid}",$values);
    $cat_vocab = taxonomy_vocabulary_load($values['vocabs']['category']);
    if (!empty($cat_vocab->machine_name)) {
      variable_set("rscl_category_{$cat_vocab->machine_name}",$nid);
    }
  
    switch($values['actions']) {
      case 'menu_rebuild':
        
        $vocab = taxonomy_vocabulary_load($values['vocabs']['category']);
        if (!empty($vocab)) {
        
          $menu_name = "rsc-library-{$nid}";
          $menus = menu_get_menus();
          if (!isset($menus[$menu_name])) { // check if our menu exists
            // create our menu
            menu_save(array(
              'menu_name'   => $menu_name,
              'title'       => $title,
              'description' => t('Auto-generated menu for a library.'),
            ));
          }
        
          // prepare form values for the taxonomy vocabulary edit form
          $form_state = array();
          $form_state['build_info']['args'] = array(&$vocab);
          $form_state['values']['taxonomy_menu']['vocab_parent'] = $menu_name.':0';
          $form_state['values']['taxonomy_menu']['options']['rebuild'] = 1;
          $form_state['values']['op'] = t('Save');
        
          // submit the taxonomy vocabulary edit form
          module_load_include('inc', 'taxonomy', 'taxonomy.admin');
          drupal_form_submit('taxonomy_form_vocabulary', $form_state);
        
        }
        
        break;
    }
  }
}

