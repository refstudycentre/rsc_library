<?php

/**
 * Implements hook_preprocess_HOOK(&$variables)
 * https://api.drupal.org/api/drupal/modules!system!theme.api.php/function/hook_preprocess_HOOK/7
 * @param $variables array() The variables for the node template.
 */
function rsc_library_preprocess_node(&$variables) {

  /*
   * For library articles:
   */

  if ($variables['type'] == 'rsc_library_article') {

    // add microdata
    $variables['attributes_array']['itemscope'] = NULL;
    $variables['attributes_array']['itemtype'] = 'http://schema.org/Article';
    $variables['title_attributes_array']['itemprop'] = 'name';

  }

  /*
   * For all rsc_library content types:
   */

  if (in_array($variables['type'], rsc_library_content_types())) {

    // find and render difficulty and author fields and merge them into a list:
    $article_infos = array();
    foreach ($variables['content'] as $field_name => &$field) {
      if (substr($field_name, 0, 12) === RSC_LIBRARY_PREFIX.'_author_') {
        $article_infos[1] = render($field);
        unset($variables['content'][$field_name]);
      } else {
        if (substr($field_name, 0, 16) === RSC_LIBRARY_PREFIX.'_difficulty_') {
          $article_infos[0] = render($field);
          unset($variables['content'][$field_name]);
        }
      }
    }
    ksort($article_infos);
    if (!empty($article_infos)) {
      $variables['content']['article_info'] = array(
        '#weight' => -20,
        '#markup' => '<ul class="article-info"><li class="first">' . implode('</li> <li>', $article_infos) . '</li></ul>',
      );
    }

    // For full view mode:
    if ($variables['view_mode'] == 'full') {

      // get some fields
      $source_field            = rsc_library_get_field('source',            $variables['content']);
      $date_field              = rsc_library_get_field('date',              $variables['content']);
      $pages_field             = rsc_library_get_field('pages',             $variables['content']);
      $editor_field            = rsc_library_get_field('editor',            $variables['content']);
      $publisher_field         = rsc_library_get_field('publisher',         $variables['content']);
      $page_range_field        = rsc_library_get_field('page_range',        $variables['content']);
      $book_title_field        = rsc_library_get_field('book_title',        $variables['content']);
      $attachment_field        = rsc_library_get_field('attachment',        $variables['content']);
      $link_to_chapter_field   = rsc_library_get_field('link_to_chapter',   $variables['content']);
      $link_to_publisher_field = rsc_library_get_field('link_to_publisher', $variables['content']);
      $link_to_buy_field       = rsc_library_get_field('link_to_buy',       $variables['content']);

      // render the fields
      $source     = render($source_field);
      $date       = render($date_field);
      $pages      = render($pages_field);
      $editor     = render($editor_field);
      $publisher  = render($publisher_field);
      $page_range = render($page_range_field);
      $book_title = render($book_title_field);

      // for library content and links
      if (in_array($variables['type'], array(
        'rsc_library_article',
        'rsc_library_link',
      ))) {

        // create compound field from source, date and pages fields
        $src = array();

        if ($source) {
          $src[] = t('Source: !src',array('!src'=>trim($source)));
        }
        if ($date) {
          $src[] = trim($date);
        }
        if ($pages) {
          $pages = t('!pages&nbsp;pages.',array('!pages'=>trim($pages)));
        }

        if ($src && $pages) {
          $variables['content']['source_info'] = array(
            '#weight' => 6,
            '#markup' => implode(', ',$src).". ".$pages,
          );
        } else if ($src) {
          $variables['content']['source_info'] = array(
            '#weight' => 6,
            '#markup' => implode(', ',$src),
          );
        } else if ($pages) {
          $variables['content']['source_info'] = array(
            '#weight' => 6,
            '#markup' => $pages,
          );
        }

      }

      // for book previews and requestable content
      if (in_array($variables['type'], array(
        'rsc_library_request',
        'rsc_library_book_preview',
      ))) {

        // create compound field from editor, publisher, date, book_title and page_range fields
        $bibliography = array();

        if ($editor) {
          $bibliography[] = t('!editor, ed.', array('!editor'=>trim($editor)));
        }

        if ($book_title) {
          $book_title = trim($book_title).' ';
        } else {
          $book_title = "";
        }

        $pub = array();
        if ($publisher) {
          $pub[] = trim($publisher);
        }
        if ($date) {
          $pub[] = trim($date);
        }
        $bibliography[] = $book_title.'('.implode(', ', $pub).')';

        if ($page_range) {
          $bibliography[] = trim($page_range);
        }

        $variables['content']['source_info'] = array(
          '#weight' => 6,
          '#markup' => t('Source: !bibliography',array('!bibliography' => implode(', ', $bibliography))),
        );

      }

      // for book previews only
      if ($variables['type'] == 'rsc_library_book_preview') {

        // make a list of links (to chapter, to publisher, and to buy)
        $book_links = array();
        if (!empty($attachment_field)) {
          // Quick fix: skip the rendering, and just get a direct link to the file, so that it looks the same as the rest of this list
          // foreach (array_keys($attachment_field['#items']) as $delta) {
          //   $book_links[] = render($attachment_field[$delta]);
          // }
          foreach($attachment_field['#items'] as $file) {
            $book_links[] = l(t("View chapter"), file_create_url($file['uri']));
          }
        }
        if (!empty($link_to_chapter_field)) {
          foreach (array_keys($link_to_chapter_field['#items']) as $delta) {
            $book_links[] = render($link_to_chapter_field[$delta]);
          }
        }
        if (!empty($link_to_publisher_field)) {
          foreach (array_keys($link_to_publisher_field['#items']) as $delta) {
            $book_links[] = render($link_to_publisher_field[$delta]);
          }
        }
        if (!empty($link_to_buy_field)) {
          foreach (array_keys($link_to_buy_field['#items']) as $delta) {
            $book_links[] = render($link_to_buy_field[$delta]);
          }
        }

        if (!empty($book_links)) {
          $variables['content']['book-links-list'] = array(
            '#weight' => 8,
            '#markup' => '<ul class="book-links-list"><li class="first">' . implode('</li><li>', $book_links) . '</li></ul>',
          );
        }

      }

      // make a list of fields (better semantics than a lot of divs)
      $exclude = array(
        'links',
        'comments',
        'article_info',
        'book-links-list',
        RSC_LIBRARY_PREFIX.'_summary',
        RSC_LIBRARY_PREFIX.'_link',
        RSC_LIBRARY_PREFIX.'_attachment',
        RSC_LIBRARY_PREFIX.'_full_text',
      );
      $items = array();
      foreach(array_diff(array_keys($variables['content']), $exclude) as $key) {
        $field = &$variables['content'][$key];
        $items[$field['#weight']] = render($field);
        unset($field);
      }
      ksort($items);
      if (!empty($items)) {
        $variables['content']['fields-list'] = array(
          '#weight' => 10,
          '#markup' => '<ul class="fields-list"><li class="first">' . implode('</li><li>', $items) . '</li></ul>',
        );
      }

    }

  }

  /*
   * For rscl_block view mode:
   */

  if ($variables['view_mode'] == RSC_LIBRARY_PREFIX.'_block') {
    // use template node--rsc-block.tpl.php
    $variables['theme_hook_suggestions'][] = "node__{$variables['view_mode']}";
  }

}


/**
 * Implements hook_preprocess_HOOK(&$variables)
 * https://api.drupal.org/api/drupal/modules!system!theme.api.php/function/hook_preprocess_HOOK/7
 * @param $variables array() The variables for the field template.
 */
function rsc_library_preprocess_field(&$variables) {

  // only act on content types defines by this module
  if (in_array($variables['element']['#bundle'], rsc_library_content_types())) {

    $field_name = $variables['element']['#field_name'];

    // Modify the display of some fields
    if ($field_name == RSC_LIBRARY_PREFIX . '_copyright_notice') {
      // assume there is only one field value
      $variables['items'][0]['#prefix'] = t("This material is made available with the permission of ");
      $variables['items'][0]['#suffix'] = t(".");
    }

// TODO: improve this bit on microdata, or move it to a separate module?
    if ($variables['element']['#bundle'] == 'rsc_library_article') {

      // microdata
      // TODO: auto-detect field-names or make them manually configurable
      switch ($variables['element']['#field_name']) {

        case 'body':
          $variables['attributes_array']['itemprop'] = 'description';
          foreach ($variables['items'] as &$item) {
            $item = preg_replace('/<a/', '<a itemprop="about"', $item);
          }
          break;

        case 'field_author_tax':
          foreach ($variables['items'] as &$item) {
            $item['#options']['attributes']['itemprop'] = 'author';
          }
          break;

        case 'field_link':
          foreach ($variables['items'] as &$item) {
            $item['#markup'] = preg_replace('/<a/', '$0 itemprop="url"', $item['#markup']);
          }
          break;

        case 'field_attachment':
          foreach ($variables['items'] as &$item) {
            $item['#markup'] = preg_replace('/<li class="read"><a/', '$0 itemprop="url"', $item['#markup']);
          }
          break;

        case 'field_source_tax':
          $variables['attributes_array']['itemprop'] = 'publisher';
          break;

        case 'field_date':
          $variables['attributes_array']['itemprop'] = 'datePublished';
          break;
      }
    }

    // Add theme hook suggestions
    if (substr($field_name, strlen(RSC_LIBRARY_PREFIX), 12) === '_difficulty_') {
      $variables['theme_hook_suggestions'][] = 'field__rscl_difficulty__';
    }
    if (substr($field_name, strlen(RSC_LIBRARY_PREFIX), 8) === '_author_') {
      $variables['theme_hook_suggestions'][] = 'field__rscl_author__';
    }
    if (substr($field_name, strlen(RSC_LIBRARY_PREFIX), 8) === '_editor_') {
      $variables['theme_hook_suggestions'][] = 'field__rscl_editor__';
    }
    if (substr($field_name, strlen(RSC_LIBRARY_PREFIX), 8) === '_source_') {
      $variables['theme_hook_suggestions'][] = 'field__rscl_source__';
    }
    if (substr($field_name, strlen(RSC_LIBRARY_PREFIX), 11) === '_publisher_') {
      $variables['theme_hook_suggestions'][] = 'field__rscl_publisher__';
    }

  }

}


/**
 * Return the first array key that starts with $str
 */
function rsc_library_find_array_key_starting_with($str, &$array) {

  $n = strlen($str);
  foreach (array_keys($array) as $key) {
    if (substr($key, 0, $n) === $str) {
      return $key;
    }
  }

}


/**
 * Find a rsc_library field
 */
function rsc_library_get_field($field_short_name, &$array, $unset=TRUE) {

  $field_name = rsc_library_find_array_key_starting_with(RSC_LIBRARY_PREFIX.'_'.$field_short_name, $array);
  $return_value = NULL;

  if (!empty($array[$field_name][0])) {
    $return_value = $array[$field_name];
  }

  if ($unset) {
    unset($array[$field_name]);
  }

  return $return_value;

}
