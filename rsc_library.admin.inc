<?php


/**
 * Form builder for the settings page
 */
function rsc_library_settings() {
  $form = array();
  
  $lid = 1; // one library id for now
  // TODO: multiple libraries
  // TODO: define library as an entity?
  
  /* collect info from site*/
  $vocabs = taxonomy_get_vocabularies();
  $vocab_options = array('-1' => t('None'));
  foreach($vocabs as $vid => $vocab) {
    $vocab_options[$vid] = $vocab->name;
  }
  
  /* collect info from this module */
  $default_settings = array(
    $lid => array(
      'title' => '',
      'vocabs' => array(
        'category'   => '-1',
        'tag'        => '-1',
        'author'     => '-1',
        'difficulty' => '-1',
      ),
    ),
  );
  $settings = variable_get('rsc_library',$default_settings);
  
  $vocab_cat = taxonomy_vocabulary_load($settings[$lid]['vocabs']['category']);
  $menu_name = "rsc_library_{$lid}";
  $menus = menu_get_menus();
  
  /* build the form */
  $form['#tree'] = TRUE; // save the whole thing as an array
  
  $links = array();
  if (!empty($vocab_cat)) {
    $links["admin/structure/taxonomy/{$vocab_cat->machine_name}/edit"] = t('Edit category vocabulary');
    $links["admin/structure/taxonomy_manager/voc/{$vocab_cat->machine_name}"] = t('Manage category terms');
  }
  $links["admin/structure/taxonomy/add"] = t('Add a new vocabulary');
  $links["admin/structure/types/manage/rsc-library-article"] = t('Edit "article" content type');
  $links["admin/structure/types/manage/rsc-library-audio"] = t('Edit "audio" content type');
  if (isset($menus[$menu_name])) {
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
  
  $form['rsc_library'][$lid]['links'] = array(
    '#type'   => 'markup',
    '#markup' => drupal_render($link_list),
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
  );
  
  $form['rsc_library'][$lid]['title'] = array(
    '#type'          => 'textfield',
    '#required'      => TRUE,
    '#multiple'      => FALSE,
    '#title'         => t('Title of the library (used for headings, etc.)'),
    '#default_value' => $settings[$lid]['title'],
  );
  
  $form['rsc_library'][$lid]['vocabs'] = array(
    '#type'        => 'fieldset',
    '#title'       => 'Taxonomy vocabulary to use for...',
  );
  
  $form['rsc_library'][$lid]['vocabs']['category'] = array(
    '#type'          => 'select',
    '#required'      => FALSE,
    '#multiple'      => FALSE,
    '#title'         => t('categories'),
    '#description'   => t('A menu will be created and populated with the terms of this vocabulary. <br/> A reference field to terms in this vocabulary will be created on library content types.'),
    '#default_value' => $settings[$lid]['vocabs']['category'],
    '#options'       => $vocab_options,
  );
  
  $form['rsc_library'][$lid]['vocabs']['author'] = array(
    '#type'          => 'select',
    '#required'      => FALSE,
    '#multiple'      => FALSE,
    '#title'         => t('authors'),
    '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
    '#default_value' => $settings[$lid]['vocabs']['author'],
    '#options'       => $vocab_options,
  );
  
  $form['rsc_library'][$lid]['vocabs']['tag'] = array(
    '#type'          => 'select',
    '#required'      => FALSE,
    '#multiple'      => FALSE,
    '#title'         => t('tags'),
    '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
    '#default_value' => $settings[$lid]['vocabs']['tag'],
    '#options'       => $vocab_options,
  );
  
  $form['rsc_library'][$lid]['vocabs']['difficulty'] = array(
    '#type'          => 'select',
    '#required'      => FALSE,
    '#multiple'      => FALSE,
    '#title'         => t('"difficulty" levels'),
    '#description'   => t('A reference field to terms in this vocabulary will be created on library content types.'),
    '#default_value' => $settings[$lid]['vocabs']['difficulty'],
    '#options'       => $vocab_options,
  );
  
  $form['rsc_library'][$lid]['actions'] = array(
    '#type'              => 'radios',
    '#title'             => 'Actions:',
    '#default_value'     => 'Nothing',
    '#options'           => array(
      0                        => 'Nothing',
      'menu_rebuild'           => '(re)Create and populate the category menu.',
      'content_type_reset'     => 'Reset the content type settings.',
      'fields_create'          => '(re)Create missing fields and instances',
      'fields_cleanup'         => 'Delete (clean up) unused fields',
    ),
    '#description'           => t('Choose an optional action to perform when submitting.'),
    'menu_rebuild'           => array('#description' => t("This might take a while!")),
    'content_type_reset'     => array('#description' => t("Default settings include: unpublished by default, comments open, etc.")),
    'fields_create'          => array('#description' => t("After changing the vocabulary settings above, first submit, then select this option and submit again. Old fields and field instances will not be deleted automatically. <br/> Also, if you are experiencing trouble with a field or field instance, delete it, and the choose this option. YOU WILL LOSE ALL VALUES CONTAINED IN EVERY INSTANCE OF A FIELD IF YOU DELETE IT.")),
    'fields_cleanup'         => array('#description' => t("This will delete all fields created by rsc_library which do not have any instances (i.e. are not linked to a content type any more). YOU WILL LOSE ALL DATA IN THESE FIELDS.")),
  );
  
  $form['#submit'][] = 'rsc_library_settings_submit';

  return system_settings_form($form);
}


/**
 * Submit handler for the settings page
 */
function rsc_library_settings_submit($form, &$form_state) {
  
  if (!empty($form_state['values']['rsc_library'])) {
    foreach ($form_state['values']['rsc_library'] as $lid => $lib_settings) {
      
      switch($lib_settings['actions']) {
        case 'menu_rebuild':
          
          $vocab = taxonomy_vocabulary_load($lib_settings['vocabs']['category']);
          if (!empty($vocab)) {
          
            $menu_name = "rsc_library_{$lid}";
            $menus = menu_get_menus();
            if (!isset($menus[$menu_name])) { // check if our menu exists
              // create our menu
              menu_save(array(
                'menu_name'   => $menu_name,
                'title'       => $lib_settings['title'],
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
}

