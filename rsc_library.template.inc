<?php

/**
 * Implements hook_preprocess_HOOK(&$variables)
 * https://api.drupal.org/api/drupal/modules!system!theme.api.php/function/hook_preprocess_HOOK/7
 */
function rsc_library_preprocess_node(&$vars) {

  if ($vars['type'] == 'rsc_library_article') { // For library articles:

    // add microdata
    // TODO: should field microdata also be done in this module? currently in theme, AFAIK ... eish
    $vars['attributes_array']['itemscope'] = NULL;
    $vars['attributes_array']['itemtype'] = 'http://schema.org/Article';
    $vars['title_attributes_array']['itemprop'] = 'name';

  }

  // For content types rsc-library-article and rsc-library-audio:
  if (in_array($vars['type'],array('rsc_library_article','rsc_library_audio'))) {

    // find and render difficulty and author fields and merge them into a list:
    $article_infos = array();
    foreach ($c as $fieldname => &$field) {
      if (substr($fieldname, 0, 12) === 'rscl_author_') {
        $article_infos[1] = render($field);
        unset($c[$fieldname]);
      }
      else {
        if (substr($fieldname, 0, 16) === 'rscl_difficulty_') {
          $article_infos[0] = render($field);
          unset($c[$fieldname]);
        }
      }
    }
    ksort($article_infos);
    if (!empty($article_infos)) {
      $c['article_info'] = array(
        '#weight' => -20,
        '#markup' => '<ul class="article-info"><li class="first">' . implode('</li> <li>', $article_infos) . '</li></ul>',
      );
    }

  }

  /*
 * For specific view modes:
 */

  switch($vars['view_mode']) {

    case 'full':

      // For content types rsc-library-article and rsc-library-audio:
      if (in_array($vars['type'],array('rsc_library_article','rsc_library_audio'))) {

        // create compound field from source, date and pages fields
        $src = array();
        $pages = "";
        // find and render source field
        foreach($c as $fieldname => &$field) {
          if (substr($fieldname,0,12) === 'rscl_source_') {
            if (!empty($field[0])) {
              $src[] = t('Source: !src',array('!src'=>trim(render($field))));
              unset($c[$fieldname]);
            }
            break; // there is only one
          }
        }
        if (!empty($c['rscl_date'][0])) {
          $src[] = trim(render($c['rscl_date']));
          unset($c['rscl_date']);
        }
        if (!empty($c['rscl_pages'][0])) {
          $pages = t('!pages&nbsp;pages.',array('!pages'=>trim(render($c['rscl_pages'][0]))));
          unset($c['rscl_pages']);
        }
        if ($src && $pages) {
          $c['source_info'] = array(
            '#weight' => -10,
            '#markup' => implode(', ',$src).". ".$pages,
          );
        } else if ($src) {
          $c['source_info'] = array(
            '#weight' => -10,
            '#markup' => implode(', ',$src),
          );
        } else if ($pages) {
          $c['source_info'] = array(
            '#weight' => -10,
            '#markup' => $pages,
          );
        }

        // make a list of fields (better semantics than a lot of divs)
        $exclude = array('links','comments','rscl_body','article_info','rscl_link','rscl_attachment','field_image');
        $items = array();
        foreach(array_diff(array_keys($c),$exclude) as $key) {
          $field = &$c[$key];
          $items[$field['#weight']] = render($field);
          unset($field);
        }
        ksort($items);
        if (!empty($items))
          $c['fields-list'] = array(
            '#weight' => 4,
            '#markup' => '<ul class="fields-list"><li class="first">'.implode('</li><li>',$items).'</li></ul>',
          );
      }

      break;

    case 'rscl_block':

      // use template node--rsc-block.tpl.php
      $vars['theme_hook_suggestions'][] = "node__{$vars['view_mode']}";

      break;

  }

}


/**
 * Implements hook_preprocess_HOOK(&$variables)
 * https://api.drupal.org/api/drupal/modules!system!theme.api.php/function/hook_preprocess_HOOK/7
 */
function rsc_library_preprocess_field(&$vars) {

  /* Modify the display of some fields */
  if ($vars['element']['#field_name'] == RSC_LIBRARY_PREFIX.'_copyright_notice') {
    // assume there is only one field value
    $vars['items'][0]['#prefix'] = t("This material is made available with the permission of ");
    $vars['items'][0]['#suffix'] = t(".");
  }

// TODO: improve this bit on microdata, or move it to a separate module?
  if ($vars['element']['#bundle'] == 'rsc_library_article') {

    // microdata
    // TODO: auto-detect field-names or make them manually configurable
    switch ($vars['element']['#field_name']) {

      case 'body':
        $vars['attributes_array']['itemprop'] = 'description';
        foreach($vars['items'] as &$item) {
          $item = preg_replace('/<a/','<a itemprop="about"',$item);
        }
        break;

      case 'field_author_tax':
        foreach ($vars['items'] as &$item) {
          $item['#options']['attributes']['itemprop'] = 'author';
        }
        break;

      case 'field_link':
        foreach ($vars['items'] as &$item) {
          $item['#markup'] = preg_replace('/<a/','$0 itemprop="url"',$item['#markup']);
        }
        break;

      case 'field_attachment':
        foreach ($vars['items'] as &$item) {
          $item['#markup'] = preg_replace('/<li class="read"><a/','$0 itemprop="url"',$item['#markup']);
        }
        break;

      case 'field_source_tax':
        $vars['attributes_array']['itemprop'] = 'publisher';
        break;

      case 'field_date':
        $vars['attributes_array']['itemprop'] = 'datePublished';
        break;
    }
  }

}