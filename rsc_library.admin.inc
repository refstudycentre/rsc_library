<?php

require_once "rsc_library.common.inc";


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
//  $default_settings = array();
  //$settings = variable_get('rsc_library',$default_settings);
  
  /* build the form */
  $form['#tree'] = TRUE; // save the whole thing as an array

  $form['rsc_library']['actions'] = array(
    '#type' => 'radios',
    '#title' => 'Actions:',
    '#default_value' => 'Nothing',
    '#options' => array(
      0 => 'Nothing',
      'content_type_reset' => 'Reset the content type settings.',
      'fields_setup' => 'Create missing fields and instances, and update existing ones.',
      'fields_cleanup' => 'Delete (clean up) unused fields',
    ),
    '#description' => t('Choose an optional action to perform when submitting.'),
    'content_type_reset' => array('#description' => t("Default settings include: unpublished by default, comments open, etc.")),
    'fields_setup' => array('#description' => t("Old fields and field instances will not be deleted automatically. If you are experiencing trouble with a field or field instance, delete it, and then choose this option. YOU WILL LOSE ALL VALUES CONTAINED IN EVERY INSTANCE OF A FIELD IF YOU DELETE IT. This will revert the widget and display settings for each field back to the values specified in code.")),
    'fields_cleanup' => array('#description' => t("This will delete all fields created by rsc_library which do not have any instances (i.e. are not linked to a content type any more). YOU WILL LOSE ALL DATA IN THESE FIELDS.")),
  );
  
  $links = array();
  $links["admin/structure/types/manage/rsc-library"] = t('Edit "library" content type');
  $links["admin/structure/types/manage/rsc-library-article"] = t('Edit "article" content type');
  $links["admin/structure/types/manage/rsc-library-link"] = t('Edit "link" content type');
  $links["admin/structure/types/manage/rsc-library-request"] = t('Edit "request" content type');
  $links["admin/structure/types/manage/rsc-library-book"] = t('Edit "book" content type');

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
        
        module_load_include('inc','rsc_library','rsc_library.field');
        rsc_library_field__cleanup();
        drupal_set_message("Unused fields from rsc_library have been deleted.");
        
        break;
      case 'fields_setup':
        
        module_load_include('inc','rsc_library','rsc_library.field');
        rsc_library_field__setup_fields();
        rsc_library_field__setup_field_instances();
        drupal_set_message("Fields and instances for rsc_library have been created and/or updated.");
        
        break;
    }
      
  }
}


/**
 * Form builder for the node settings page
 * @param stdClass|null $node The node being edited, or null when creating a new node
 * @return array The form array
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
    $settings = variable_get("rsc_library_{$node->nid}", array());
    
    $vocab_cat = isset($settings['vocabs']['category']) ? taxonomy_vocabulary_load($settings['vocabs']['category']) : null;
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
      '#default_value' => isset($settings['vocabs']['category']) ? $settings['vocabs']['category'] : -1,
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['author'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('authors'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => isset($settings['vocabs']['author']) ? $settings['vocabs']['author'] : -1,
      '#options'       => $vocab_options,
    );

    $form['vocabs']['editor'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('editors'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => isset($settings['vocabs']['editor']) ? $settings['vocabs']['editor'] : -1,
      '#options'       => $vocab_options,
    );

    $form['vocabs']['publisher'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('publishers'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => isset($settings['vocabs']['publisher']) ? $settings['vocabs']['publisher'] : -1,
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['source'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('sources'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => isset($settings['vocabs']['source']) ? $settings['vocabs']['source'] : -1,
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['tag'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('tags'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => isset($settings['vocabs']['tag']) ? $settings['vocabs']['tag'] : -1,
      '#options'       => $vocab_options,
    );
  
    $form['vocabs']['difficulty'] = array(
      '#type'          => 'select',
      '#required'      => FALSE,
      '#multiple'      => FALSE,
      '#title'         => t('"difficulty" levels'),
      '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
      '#default_value' => isset($settings['vocabs']['difficulty']) ? $settings['vocabs']['difficulty'] : -1,
      '#options'       => $vocab_options,
    );

    $form['vocabs']['permission_reminder'] = array(
      '#type'   => 'markup',
      '#markup' => '<p>Remember to give the "RSC library staff" role permissions to add, edit and/or delete terms in these vocabularies.</p>'
    );

//    $form['urls'] = array(
//      '#type' => 'textarea',
//      '#title' => t("Additionally show this library's blocks on"),
//      '#default_value' => isset($settings['urls']) ? implode(PHP_EOL, $settings['urls']) : array(),
//      '#description' => t("Normally, library blocks such as 'featured nodes' and 'new nodes' are only displayed on the library front page, category pages, and node pages that belong to the library. Here, you can specify additional paths which should be considered part of the library. Enter one path per line. The '*' character is a wildcard."),
//    );
  
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
    $form['#markup'] = "<p>Save the library and edit it again to see more options ...</p><p><small>This is because the library node needs a 'nid' before we can save the relevant variables.</small></p>";
  }
  
  return $form;

}


/**
 * Submit handler for the node settings page
 */
function rsc_library_node_settings_submit($form, &$form_state) {

  $values = &$form_state['values']['rsc_library_node_settings'];

  /* Handle actions */

  $title = $form_state['values']['title'];
  $nid = $form_state['values']['nid'];
  if (is_numeric($nid) && is_array($values) && isset($values['vocabs'])) {
    variable_set("rsc_library_{$nid}",$values);
    $cat_vocab = taxonomy_vocabulary_load($values['vocabs']['category']);
    if (!empty($cat_vocab->machine_name)) {
      variable_set(RSC_LIBRARY_PREFIX."_category_{$cat_vocab->machine_name}",$nid);
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

