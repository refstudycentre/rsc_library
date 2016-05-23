<?php

// Field names are limited to 32 characters, so we use a short prefix:
define('RSC_LIBRARY_PREFIX', 'rscl');


/**
 * Content types. Used by hook_node_info, among others.
 */
function rsc_library_content_types_info() {
  return array(
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
  );
}


function rsc_library_content_types() {
  return array(
    'rsc_library_article',
    'rsc_library_link',
    'rsc_library_request',
    'rsc_library_book_preview',
  );
}


/**
 * (re)set content type settings
 */
function rsc_library_content_type_settings() {

  // for each "rsc_library_*" content type
  foreach (rsc_library_content_types() as $type) {
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
  variable_set("pathauto_node_rsc_library_book_pattern", 'book-preview/[node:title]');
  
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

